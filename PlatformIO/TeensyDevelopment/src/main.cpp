#include <Arduino.h>

// #include "EVT_StateMachine.h"
// #include "EVT_VescDriver.h"
// #include "EVT_Ethernet.h"
// #include "EVT_AutoMode.h"
// #include "EVT_ODriver.h"

#include "TransmitterConstants.hpp"
#include "ConversionConstants.hpp"
#include "ModuleConstants.hpp"
#include "IOConstants.hpp"
using namespace Constants;

// NONE,   < No state defined.
// INIT,   < Initialization state.
// IDLE,   < Idle state.  
// CALIB,  < Calibration state.
// RC,     < Remote Control state.
// AUTO,   < Autonomous state.
// ERR,    < Error state.

unsigned long currentTime = 0UL;
unsigned long lastUpdate = 0UL;


/**
 * @brief One time setup code
 */
void setup() {
  // Begin the serial monitor for the teensy output
  Serial.begin(IOConstants::serialBaudrate);

  // Set the default mapping for the RC transmitter
  ModuleConstants::transmitter.setMapping(TransmitterConstants::defaultJoystick, Signals::ControlRC::mapType::JOYSTICK);
  ModuleConstants::transmitter.setMapping(TransmitterConstants::defaultSwitch, Signals::ControlRC::mapType::SWITCH);
  ModuleConstants::transmitter.setMapping(TransmitterConstants::defaultTriSwitch, Signals::ControlRC::mapType::TRI_SWITCH);
  ModuleConstants::transmitter.setMapping(TransmitterConstants::defaultKnob, Signals::ControlRC::mapType::KNOB);

  // Set the contactor relays to OUTPUT pin mode
  pinMode(IOConstants::oDriveRelay, OUTPUT);
  pinMode(IOConstants::eBrakeRelay, OUTPUT);
  pinMode(IOConstants::vescRelay, OUTPUT);

  // Set the LED relays to OUTPUT pin mode
  pinMode(IOConstants::redLedRelay, OUTPUT);
  pinMode(IOConstants::greenLedRelay, OUTPUT);
  pinMode(IOConstants::yellowLedRelay, OUTPUT);

  // Power on ODrive contactor
  digitalWrite(IOConstants::oDriveRelay, HIGH);

  // Perform the initial ODrive setup check
  ModuleConstants::odrive.setup();

  /*
  SetState(NONE);
  
  // Set the car into initialization state 
  SetState(INIT); 

  // Power on all the contactors 
  Serial.println("Powering up contactors...");
  digitalWrite(IOConstants::oDriveRelay, HIGH);
  digitalWrite(IOConstants::eBrakeRelay, HIGH);
  digitalWrite(IOConstants::vescRelay, HIGH);
  delay(1'000);
  
  // Initialize all the modules 
  Serial.println("Initializing modules...");
  setupSbus();
  setupVesc();
  setupOdrv();
  //setupTelemetryUDP();
  
  // Set the car into IDLE state 
  SetState(IDLE);
  */

  // Get the current time in milliseconds for timing
  currentTime = millis();
}


/**
 * @brief Code to run continusously on runtime
 */
void loop() {
  /*
  Serial.println("Main loop iteration");
  loop_count++;

  updateSbusData(); // reads the RC reciever to get sbus data
  // sendTelemetry(); // sends telemetry data over UDP to panda

  // add switches to corresponding RC channels here
  auto_switch = channels[6];
  calibration_switch = channels[5]; // just for calibration out of idle on starup and starts RC
  reset_switch = channels[4]; // runs odrive calibration and clears errors from err state

  switch (GetState()) {
    case RC:
      Serial.println("In RC Control Mode");
      updateSbusData();
    
      if (auto_switch > 1000) {
        SetState(AUTO);
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
      digitalWrite(IOConstants::oDriveRelay, LOW); // Turn off ODrive relay
      digitalWrite(IOConstants::eBrakeRelay, LOW); // Turn off E-Brake relay 
      digitalWrite(IOConstants::vescRelay, LOW);   // Turn off VESC relay 

      // check for reset
      if (reset_switch > 1000){

        digitalWrite(IOConstants::oDriveRelay, HIGH); // Turn on ODrive relay
        digitalWrite(IOConstants::eBrakeRelay, HIGH); // Turn on E-Brake relay 
        digitalWrite(IOConstants::vescRelay, HIGH);   // Turn on VESC relay 
        Serial.println("Attempting to clear errors...");

        if (auto_switch > 1'000) {
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
  if (reset_switch > 1000 && auto_switch < 1000){
    Serial.println("Reset switch activated. Returning to IDLE state.");
    SetState(IDLE);
  }
  */

  if ((currentTime - lastUpdate) >= (ConversionConstants::secToMillis / IOConstants::updateFrequency)) {
    ModuleConstants::odrive.updateRC();

    lastUpdate = millis();
  }

  currentTime = millis();

  delay(10);
}
