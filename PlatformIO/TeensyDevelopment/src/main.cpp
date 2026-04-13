#include <Arduino.h>

#include "EVT_StateMachine.h"
#include "EVT_VescDriver.h"
#include "EVT_AutoMode.h"

#include <EVT_Ethernet.hpp>
#include <EVT_ODriver.hpp>
#include <EVT_RC.hpp>

#include "TransmitterConstants.hpp"
#include "ConversionConstants.hpp"
#include "ModuleConstants.hpp"
#include "IOConstants.hpp"
using namespace Constants;

// bool autonomous = false;
// int loop_count = 0;
// int loops_per_telem = 10;

// NONE,   < No state defined.
// INIT,   < Initialization state.
// IDLE,   < Idle state.  
// CALIB,  < Calibration state.
// RC,     < Remote Control state.
// AUTO,   < Autonomous state.
// ERR,    < Error state.

unsigned long currentTime = 0UL;
unsigned long lastUpdate = 0UL;
unsigned long lastWaitingPrintMs = 0UL;


/**
 * @brief One time setup code
 */
void setup() {
  // Begin the serial monitor for the teensy output
  Serial.begin(IOConstants::serialBaudrate);

  pinMode(IOConstants::ledBuiltIn, OUTPUT);

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
  digitalWrite(IOConstants::eBrakeRelay, HIGH);
  digitalWrite(IOConstants::vescRelay, HIGH);


  // Perform the initial ODrive setup check
  ModuleConstants::odrive.setup();

  SetState(NONE);
  
  // Set the car into initialization state 
  SetState(INIT); 

  ModuleConstants::ethernet.setupUDP();

  
  SetState(NONE);

  // Set the car into initialization state
  SetState(INIT);

  //Initialize all the modules
  Serial.println("Initializing modules...");
  setupVesc();
  

  // Get the current time in milliseconds for timing
  currentTime = millis();
  lastUpdate = currentTime;
  lastWaitingPrintMs = currentTime;
}


/**
 * @brief Code to run continusously on runtime
 */
void loop() {
  /*
  Serial.println("Main loop iteration");
  loop_count++;

  updateSbusData(); // reads the RC reciever to get sbus data
  sendTelemetry(); // sends telemetry data over UDP to panda

  // add switches to corresponding RC channels here
  auto_switch = channels[6];
  calibration_switch = channels[5]; // just for calibration out of idle on starup and starts RC
  reset_switch = channels[4];       // runs odrive calibration and clears errors from err state

  switch (GetState()) {
    case RC:
      Serial.println("In RC Control Mode");
      updateSbusData();

      if (auto_switch > 1'000) {
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

      if (auto_switch < 1'000) {
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
      if (reset_switch > 1'000) {

        digitalWrite(IOConstants::oDriveRelay, HIGH); // Turn on ODrive relay
        digitalWrite(IOConstants::eBrakeRelay, HIGH); // Turn on E-Brake relay
        digitalWrite(IOConstants::vescRelay, HIGH);   // Turn on VESC relay
        Serial.println("Attempting to clear errors...");

        if (auto_switch > 1'000) {
          Serial.println("TURN OFF AUTO SWITCH BEFORE ATTEMPTING TO CLEAR ERRORS");
        } else {
          Serial.println();
          Serial.println("yay! Errors cleared :3");
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
      if (calibration_switch > 400 && auto_switch < 1'000) {
        SetState(RC);
      } else {
        updateSbusData();

        if (auto_switch > 1'000) {
          Serial.println("[Auto Switch is on ya dingus]");
        }

        delay(1'000); // Add a delay to avoid flooding the serial output
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
  if (reset_switch > 1'000 && auto_switch < 1'000) {
    Serial.println("Reset switch activated. Returning to IDLE state.");
    SetState(IDLE);
  }
  */

  const bool gotValidFrame = ModuleConstants::transmitter.update();
  currentTime = millis();

  if (!gotValidFrame) {
    if ((currentTime - lastWaitingPrintMs) >= 1'000UL) {
      Serial.println("Waiting for valid SBUS frame...");
      lastWaitingPrintMs = currentTime;
    }
    return;
  }

  // Keep SBUS handling in the RC module; during INIT, wait for SWA-triggered
  // ODrive calibration before transitioning to RC.
  if ((currentTime - lastUpdate) >= (ConversionConstants::secToMillis / IOConstants::updateFrequency)) {
    digitalWrite(IOConstants::ledBuiltIn, HIGH);
    const uint16_t resetInput =
        ModuleConstants::transmitter.getChannelValue(Signals::ChannelRC::SWH, false);
    const uint16_t rcInput =
        ModuleConstants::transmitter.getChannelValue(Signals::ChannelRC::SWF, false);

    if (GetState() == RC && resetInput > 1500) {
      Serial.println("SWH ▶ IDLE");
      SetState(IDLE);
    }

    if (GetState() == INIT) {
      ModuleConstants::odrive.updateRC();

      if (ModuleConstants::odrive.isCalibrated()) {
        SetState(RC);
      }
    } else if (GetState() == IDLE) {
      if (resetInput < 1500 && rcInput > 900) {
        Serial.println("SWF ▶ RC");
        SetState(RC);
      }
    } else if (GetState() == RC) {
      ModuleConstants::odrive.updateRC();
      updateVescControl();
    }

    ModuleConstants::ethernet.sendTelemetry();
    lastUpdate = currentTime;
    lastWaitingPrintMs = currentTime;
  }
}
