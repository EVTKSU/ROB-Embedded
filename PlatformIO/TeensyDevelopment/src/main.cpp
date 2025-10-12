#include <Arduino.h>

#include "EVT_StateMachine.h"
#include "EVT_VescDriver.h"
#include "EVT_Ethernet.h"
#include "EVT_AutoMode.hpp"
#include "EVT_ODriver.h"
#include "EVT_RC.hpp"

/** 
 * Values of the STATE enum:
 *   NONE  -- No state defined
 *   INIT  -- Initialization state
 *   IDLE  -- Idle state
 *   CALIB -- Calibration state
 *   RC    -- Remote Control state
 *   AUTO  -- Autonomous state
 *   ERR   -- Error state
**/

uint16_t auto_switch;        // Current value of the auto switch as an int
uint16_t calibration_switch; // Current value of the calibration switch as an int
uint16_t reset_switch;       // Current value of the reset switch as an int
EVT_RC rc;
AutoMode auto_mode; 

int loop_count = 0;       // Number of calls to loop
int loops_per_telem = 10; // Number of loops between telemetry messages

const int ODriveRelayPin = 3;
const int VSECRelayPin = 4;
const int ContactorRelayPin = 5; 


/**
 * @brief One time setup to run during Teensy start up
 */
void setup() {
  SetState(STATE::NONE); // Set the state to NONE while doing initial setup

  Serial.begin(9600);
  while (!Serial) { delay(20); } // Wait for Serial Monitor to open
  
  // Initialize modules
  SetState(STATE::INIT);     // Set state to INIT after Serial finishes initializing
  pinMode(ODriveRelayPin, OUTPUT);    // Set the ODrive relay pin to ouput
  pinMode(VSECRelayPin, OUTPUT);      // Set the VESC relay pin to output
  pinMode(ContactorRelayPin, OUTPUT); // Set the Contactor relay pin to output

  Serial.println("Powering up contactors...");
  digitalWrite(ODriveRelayPin, HIGH);    // Turns on the ODrive relay
  digitalWrite(VSECRelayPin, HIGH);      // Turns on the VESC relay
  digitalWrite(ContactorRelayPin, HIGH); // Turns on the Contactor relay
  delay(1000);
  
  Serial.println("Initializing modules...");
  setupTelemetryUDP();   // Set up communication between Teensy and Panda
  rc.setupSbus();           // Set up communication between RC Transmitter and Teensy
  setupVesc();           // Set up communication between Teensy and VESCs
  setupOdrv();           // Set up communication between Teensy and ODrive

  SetState(STATE::IDLE); // Set the car into IDLE before the first call to loop
}


/**
 * @brief Main code loop 
 */
void loop() {
  loop_count++;
  rc.updateSbusData(); // Reads the RC reciever to get SBUS data
  sendTelemetry();  // Sends telemetry data over UDP to Panda 

  // Update the values of the switches from the SBUS channel data 
  auto_switch = channels[6];
  calibration_switch = channels[5]; // Used for calibration out of IDLE, on start-up, and starts RC
  reset_switch = channels[4];       // Runs ODrive calibration and clears errors from ERR

  switch (GetState()) {
    case (STATE::RC):
      rc.updateSbusData();

      if (auto_switch > 1000) { // Goes into AUTO state if the auto switch is pulled
        SetState(STATE::AUTO);
      } else { // Updates the control values if the auto switch isn't pulled
        updateVescControl();
        updateOdrvControl();
        loops_per_telem = 10;
      }

      break;
    case (STATE::AUTO):
      if (auto_switch < 1000) { // Goes into IDLE state if auto switch is released
        SetState(STATE::IDLE);
      } else { // Continues in AUTO state as long as the switch is kept held
        auto_mode.updateAutonomousMode();
        loops_per_telem = 1;
      }

      break;
    case (STATE::ERR):
      digitalWrite(ODriveRelayPin, LOW);    // Turn off ODrive relay
      digitalWrite(VSECRelayPin, LOW);      // Turn off VESC relay
      digitalWrite(ContactorRelayPin, LOW); // Turn off Contactor relay

      // Check for reset
      if (reset_switch > 1000){
        digitalWrite(ODriveRelayPin, HIGH);    // Turn on ODrive relay
        digitalWrite(VSECRelayPin, HIGH);      // Turn on VESC relay
        digitalWrite(ContactorRelayPin, HIGH); // Turn on Contactor relay

        Serial.println("Attempting to clear errors...");
        if (auto_switch > 1000) { // Doesn't clear errors if the auto switch is held 
          Serial.println("TURN OFF AUTO SWITCH BEFORE ATTEMPTING TO CLEAR ERRORS");
        } else { // Allows errors to be cleared when auto switch is released
          Serial.println();
          Serial.println("Yay! Errors cleared :D");

          SetState(STATE::IDLE); // Goes into IDLE state
          odrive.setState(ODriveAxisState::AXIS_STATE_UNDEFINED); // Sets the ODrive to undefined state 
        }
      }
      
      break;
    case (STATE::IDLE):
      // Updates the SBUS data
      rc.updateSbusData();
      loops_per_telem = 30;

      // Check if the system is in IDLE and not in ERR. If in IDLE, wait for commands
      if (calibration_switch > 400 && auto_switch < 1000) {
        SetState(STATE::RC); // Goes into RC if calibrated and not in AUTO
      } else {
        rc.updateSbusData();
        
        if (auto_switch > 1000) {
          Serial.println("[Auto Switch is on ya dingus]");
        }
        
        delay(1000); // Add a delay to avoid flooding the serial output
      }

      break;
    default:
      // Default into IDLE if an unknown state is received
      Serial.println("Warning: Unknown state encountered. Defaulting to IDLE.");
      SetState(STATE::IDLE);
      PrintState();

      break;
  }

  // Update SBUS data to receive any 
  rc.updateSbusData();

  // Go into IDLE if the reset switch is pulled
  if (reset_switch > 1000 && auto_switch < 1000){
    SetState(STATE::IDLE);
  }
}