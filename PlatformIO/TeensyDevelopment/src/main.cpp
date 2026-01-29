#include <Arduino.h>

#include "EVT_RC.h"
#include "EVT_ODriver.h"
#include "EVT_AutoMode.h"
#include "EVT_Ethernet.h"
#include "EVT_VescDriver.h"
#include "EVT_StateMachine.h"

#include <EVT_FanController.hpp>

#include <IOConstants.hpp>

using namespace Constants;

// NONE,   < No state defined.
// INIT,   < Initialization state.
// IDLE,   < Idle state.  
// CALIB,  < Calibration state.
// RC,     < Remote Control state.
// AUTO,   < Autonomous state.
// ERR,    < Error state.

uint16_t auto_switch;
uint16_t calibration_switch; 
uint16_t reset_switch;

bool autonomous = false;
int loop_count = 0;
int loops_per_telem = 10;

const bool printFanOutput = false;
const double tempMeasureFrequency = 1;
unsigned long lastTempTime;

FanController fan; // Fan module control object


/**
 * @brief Initial code setup
 */
void setup() {
  SetState(NONE);
  Serial.begin(9600);
  delay(1000); // Wait for Serial Monitor to open
  
  // Initialize modules.
  SetState(INIT);

  pinMode(IOConstants::odriveRelay, OUTPUT); // ODrive relay
  pinMode(IOConstants::eBrakeRelay, OUTPUT); // E-Brake really
  pinMode(IOConstants::vescRelay, OUTPUT); // VESC relay

  pinMode(IOConstants::redLedRelay, OUTPUT); // Red LED relay
  pinMode(IOConstants::greenLedRelay, OUTPUT); // Green LED relay
  pinMode(IOConstants::yellowLedRelay, OUTPUT); // Yellow LED relay

  Serial.println("Powering up contactors...");
  digitalWrite(IOConstants::odriveRelay, HIGH);
  digitalWrite(IOConstants::eBrakeRelay, HIGH);
  digitalWrite(IOConstants::vescRelay, HIGH);
  delay(1000);

  // fan.updateSensorData();
  // fan.propTempControl();
  
  Serial.println("Initializing modules...");
  setupSbus();
  setupVesc();
  setupOdrv();
  setupTelemetryUDP();
  SetState(IDLE);
  Serial.println("i made it here.");
}


/**
 * @brief Main code loop 
 */
void loop() {
  Serial.println("Main loop iteration");
  loop_count++;
  updateSbusData(); // reads the RC reciever to get sbus data
  //sendTelemetry(); // sends telemetry data over UDP to panda 

  // add switches to corresponding RC channels here
  auto_switch = channels[6];
  calibration_switch = channels[5]; // just for calibration out of idle on starup and starts RC
  reset_switch = channels[7]; // runs odrive calibration and clears errors from err state

  // Measure temperature and update fan speed
  // if (millis() - lastTempTime >= (1000 / tempMeasureFrequency)) {
  //   fan.updateSensorData();
  //   fan.propTempControl();

  //   if (Serial && printFanOutput) {
  //     Serial.printf(
  //       "Temp: %0.2fº\t|\tSpeed: %0.2f\n",
  //       fan.getTemperature(),
  //       fan.getFanPWMValue()
  //     );
  //   }
  // }


  switch (GetState()) {
    case RC:
      Serial.println("In RC Control Mode");

      updateSbusData();
      
      if (auto_switch > 1000) {
        SetState(AUTO);
      } else if (reset_switch > 1000) {
        SetState(IDLE);
        Serial.println("Reset switch activated, returning to IDLE.");
      } else {
        updateVescControl();
        updateOdrvControl();
        loops_per_telem = 10;
      }

      break;
    case AUTO:
      if (autonomous == false) {
        Serial.println("Entering Autonomous Mode");
        odrive_serial.println("w axis0.trap_traj.config.accel_limit " + String(1300));
        odrive_serial.println("w axis0.trap_traj.config.decel_limit " + String(1300));
      }

      if (auto_switch < 1000) {
        Serial.println("auto switch is off in case auto");
        SetState(IDLE);
      } else {
        autonomous = true;
        Serial.println("I'm Autonomizing it");
        updateAutonomousMode();
        loops_per_telem = 1;
      }

      break;
    case ERR:
      digitalWrite(IOConstants::odriveRelay, LOW); // Turn off relay 1 (odrive)
      digitalWrite(IOConstants::eBrakeRelay, LOW); // Turn off relay 2 (vesc)
      digitalWrite(IOConstants::vescRelay, LOW);   // Turn off relay 3 (contactor)

      digitalWrite(IOConstants::redLedRelay, HIGH); // Turn on the red LED

      // check for reset
      if (reset_switch > 1000){
        digitalWrite(IOConstants::odriveRelay, HIGH); // Turn on relay 1 (odrive)
        digitalWrite(IOConstants::eBrakeRelay, HIGH); // Turn on relay 2 (vesc)
        digitalWrite(IOConstants::vescRelay, HIGH); // Turn on relay 3 (contactor)

        Serial.println("Attempting to clear errors...");
        if (auto_switch > 1000) {
          Serial.println("TURN OFF AUTO SWITCH BEFORE ATTEMPTING TO CLEAR ERRORS");
        } else {
          Serial.println();
          Serial.println("yay! Errors cleared :D");
          SetState(IDLE);
          odrive.setState(AXIS_STATE_UNDEFINED);
        }
      }

      break;
    case IDLE:
      updateSbusData();

      Serial.println("System is IDLE. Waiting for commands...");
      Serial.print(channels[5]);
      loops_per_telem = 30;

      // Check if the system is idle and not in error state. if idle, it waits for commands.
      if (calibration_switch > 400 && auto_switch < 1000) {
        SetState(RC);
      } else {
        updateSbusData();
        
        if (auto_switch > 1000) {
          Serial.println("[Auto Switch is on ya dingus]");
        }
        
        delay(1000); // Add a delay to avoid flooding the serial output
      }

      break;
    default:
      Serial.println("Warning: Unknown state encountered. Defaulting to IDLE.");
      SetState(IDLE);
      PrintState();

      break;
  }

  updateSbusData();

  // if reset is ever on it puts us in idle
  if (reset_switch > 1000){
    SetState(IDLE);
    Serial.println("Reset switch activated, returning to IDLE.");
  }

  // if (loops_per_telem % loop_count == 0){
  //   sendTelemetry();
  // }
}
