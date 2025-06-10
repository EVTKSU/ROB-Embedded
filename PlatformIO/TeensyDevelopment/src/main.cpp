#include <Arduino.h>
#include "EVT_Ethernet.h"
#include "EVT_RC.h"
#include "EVT_VescDriver.h"
#include "EVT_ODriver.h"
#include "EVT_AutoMode.h"
#include "EVT_StateMachine.h"

    // NONE,   < No state defined.
    // INIT,   < Initialization state.
    // IDLE, < Idle state.  
    // CALIB,  < Calibration state.
    // RC,     < Remote Control state.
    // AUTO,   < Autonomous state.
    // ERR,     < Error state.

  uint16_t auto_switch;
  uint16_t calibration_switch; 
  uint16_t reset_switch;

  int loop_count = 0;
  int loops_per_telem = 10;

void setup() {
  SetState(NONE);
  Serial.begin(9600);
  delay(1000); // Wait for Serial Monitor to open
  
  // Initialize modules.
  SetState(INIT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  Serial.println("Powering up contactors...");
  digitalWrite(3, HIGH);
  digitalWrite(4, HIGH);
  digitalWrite(5, HIGH);
  delay(1000);
  
  Serial.println("Initializing modules...");
  setupTelemetryUDP();
  setupSbus();
  setupVesc();
  setupOdrv();
  SetState(IDLE);
}

void loop() {

  loop_count++;
  updateSbusData(); // reads the RC reciever to get sbus data
  sendTelemetry();// sends telemetry data over UDP to panda 
  // add switches to corresponding RC channels here
  auto_switch = channels[6];
  calibration_switch = channels[5]; // just for calibration out of idle on starup and starts RC
  reset_switch = channels[4]; // runs odrive calibration and clears errors from err state

  // old main loop
  // If RC data is available and channel 6 exceeds the threshold, run autonomous mode.

  // if (channels[6] > 1000) {
  //   updateAutonomousMode();
  // } else {
  //   updateVescControl();
  //   updateOdrvControl();
  // }

  switch (GetState())
  {
    case RC:

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
      if (auto_switch < 1000) {
        SetState(IDLE);
      } else {

        updateAutonomousMode();
        loops_per_telem = 1;
      }
      break;

    case ERR:
        digitalWrite(3, LOW); // Turn off relay 1 (odrive)
        digitalWrite(4, LOW); // Turn off relay 2 (vesc)
        digitalWrite(5, LOW); // Turn off relay 3 (contactor)
      // check for reset
      if (reset_switch > 1000){

        digitalWrite(3, HIGH); // Turn on relay 1 (odrive)
        digitalWrite(4, HIGH); // Turn on relay 2 (vesc)
        digitalWrite(5, HIGH); // Turn on relay 3 (contactor)
        Serial.println("Attempting to clear errors...");
        if (auto_switch > 1000) {
          Serial.println("TURN OFF AUTO SWITCH BEFORE ATTEMPTING TO CLEAR ERRORS");
        }else{
          Serial.println();
          Serial.println("yay! Errors cleared :D");
          SetState(IDLE);
          odrive.setState(AXIS_STATE_UNDEFINED);
        }
      }
      break;
    
    case IDLE:
        updateSbusData();
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
    SetState(IDLE);
  }

  // if (loops_per_telem % loop_count == 0){
  //   sendTelemetry();
  // }

}
