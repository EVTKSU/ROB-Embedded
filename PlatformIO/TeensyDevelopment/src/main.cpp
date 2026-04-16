#include <Arduino.h>

#include "EVT_AutoMode.h"

#include <EVT_Ethernet.hpp>
#include <EVT_RC.hpp>

#include "TransmitterConstants.hpp"
#include "ConversionConstants.hpp"
#include "ModuleConstants.hpp"
#include "IOConstants.hpp"
using namespace Constants;

unsigned long currentTime = 0UL;      // Current timestamp in milliseconds
unsigned long lastUpdate = 0UL;       // Timestamp in milliseconds of the last attempted update to all values 
unsigned long lastWaitingPrint = 0UL; // Timestamp in milliseconds of the last serial output denoting a bad frame
unsigned long lastPrintMessage = 0UL; // Timestamp in milliseconds of the last serial output

bool ledState = false;  // Current state of the on board LED

bool resetInput;        // Input value as a boolean from the reset channel (SWH)
bool rcInput;           // Input value as a boolean from the RC toggle channel (SWF)

uint16_t throttleInput; // Throttle input for VESC 


/**
 * @brief One time setup code
 */
void setup() {
  // Begin the serial monitor for the teensy output
  Serial.begin(IOConstants::serialBaudrate);

  // Set the on board LED pin to OUTPUT
  pinMode(IOConstants::ledBuiltIn, OUTPUT);
  digitalWrite(IOConstants::ledBuiltIn, HIGH);

  // Set the default mapping for the RC channels 
  ModuleConstants::transmitter.setMapping(TransmitterConstants::defaultJoystick, Signals::ControlRC::mapType::JOYSTICK);
  ModuleConstants::transmitter.setMapping(TransmitterConstants::defaultSwitch, Signals::ControlRC::mapType::SWITCH);
  ModuleConstants::transmitter.setMapping(TransmitterConstants::defaultTriSwitch, Signals::ControlRC::mapType::TRI_SWITCH);
  ModuleConstants::transmitter.setMapping(TransmitterConstants::defaultKnob, Signals::ControlRC::mapType::KNOB);

  // Sets the behavior of the NONE state 
  ModuleConstants::stateMachine.defineState(Signals::States::NONE, [&] () {
    ModuleConstants::light.setColorState(IOConstants::yellow);

    if ((currentTime - lastUpdate) >= (ConversionConstants::secToMillis / (2 * IOConstants::ledBlinkFrequency))) {
      digitalWrite(IOConstants::ledBuiltIn, ledState = !ledState ? HIGH : LOW);
    }
  });

  // Sets the behavior of the IDLE state
  ModuleConstants::stateMachine.defineState(Signals::States::IDLE, [&] () {
    ModuleConstants::light.setColorState(IOConstants::yellow, true, 1.0);

    if (ModuleConstants::transmitter.getChannelValue(Signals::ChannelRC::SWF, Signals::ControlRC::mapSwitches) && !ModuleConstants::transmitter.getChannelValue(Signals::ChannelRC::SWH, Signals::ControlRC::mapSwitches)) {
      ModuleConstants::stateMachine.setState(Signals::States::RC);
    }
  });

  // Sets the behavior of the RC state
  ModuleConstants::stateMachine.defineState(Signals::States::RC, [&] () {
    ModuleConstants::light.setColorState(IOConstants::green);

    if (ModuleConstants::transmitter.getChannelValue(Signals::ChannelRC::SWD, Signals::ControlRC::mapSwitches)) {
      ModuleConstants::stateMachine.setState(Signals::States::AUTO);
    } else if (ModuleConstants::transmitter.getChannelValue(Signals::ChannelRC::SWH, Signals::ControlRC::mapSwitches)) {
      ModuleConstants::stateMachine.setState(Signals::States::RESET);
    } else {
      ModuleConstants::odrive.updateRC();
      ModuleConstants::vesc.updateRC(throttleInput);
    }
  });

  // Sets the behavior of the AUTO state
  ModuleConstants::stateMachine.defineState(Signals::States::AUTO, [&] () {
    ModuleConstants::light.setColorState(IOConstants::green, true, 1.0);
  });

  // Sets the behavior of the ERROR state
  ModuleConstants::stateMachine.defineState(Signals::States::ERROR, [&] () {
    ModuleConstants::light.setColorState(IOConstants::red);

    Serial.println("An error occurred");
    Serial.println("Toggle SWH to reset");

    if (ModuleConstants::transmitter.getChannelValue(Signals::ChannelRC::SWH, Signals::ControlRC::mapSwitches)) {
      ModuleConstants::stateMachine.setState(Signals::States::RESET);
    }

    digitalWrite(IOConstants::oDriveRelay, LOW);
    digitalWrite(IOConstants::eBrakeRelay, LOW);
    digitalWrite(IOConstants::vescRelay, LOW);
  });

  // Sets the behavior of the STOP state
  ModuleConstants::stateMachine.defineState(Signals::States::STOP, [&] () {
    ModuleConstants::light.setColorState(IOConstants::red, true, 1.0);
  });

  // Sets the behavior of the RESET state
  ModuleConstants::stateMachine.defineState(Signals::States::RESET, [&] () {
    ModuleConstants::light.setColorState(IOConstants::colorOff);
  });

  // Set the contactor relays to OUTPUT pin mode
  pinMode(IOConstants::oDriveRelay, OUTPUT);
  pinMode(IOConstants::eBrakeRelay, OUTPUT);
  pinMode(IOConstants::vescRelay, OUTPUT);

  // Power on ODrive contactor
  digitalWrite(IOConstants::oDriveRelay, HIGH);
  digitalWrite(IOConstants::eBrakeRelay, HIGH);
  digitalWrite(IOConstants::vescRelay, HIGH);

  // Perform the initial setup
  Serial.println("Initializing modules...");
  ModuleConstants::odrive.setup();
  ModuleConstants::ethernet.setupUDP();

  // Set the car into idle state
    ModuleConstants::stateMachine.setState(Signals::States::IDLE);

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

  // Keep SBUS handling in the RC module
  // Wait for SWA-triggered ODrive calibration before transitioning to RC
  if ((currentTime - lastUpdate) >= (ConversionConstants::secToMillis / IOConstants::updateFrequency)) {
    if (!ModuleConstants::transmitter.update()) {
      if ((currentTime - lastWaitingPrint) >= 1'000UL) {
        Serial.println("Waiting for valid SBUS frame...");
        lastWaitingPrint = millis();
      }

      return;
    }
    
    resetInput = ModuleConstants::transmitter.getChannelValue(Signals::ChannelRC::SWH, false);
    rcInput = ModuleConstants::transmitter.getChannelValue(Signals::ChannelRC::SWF, false);
    throttleInput = ModuleConstants::transmitter.getChannelValue(Signals::ChannelRC::RIGHT_Y, false);

    if (ModuleConstants::stateMachine.isInState(Signals::States::RC) && resetInput) {
      Serial.println("SWH -> IDLE");
      ModuleConstants::stateMachine.setState(Signals::States::IDLE);

      ModuleConstants::odrive.reset();
    }

    if (ModuleConstants::stateMachine.isInState(Signals::States::IDLE) && (rcInput && !resetInput)) {
      Serial.println("SWF -> RC");
      ModuleConstants::stateMachine.setState(Signals::States::RC);
    } else if (ModuleConstants::stateMachine.isInState(Signals::States::RC)) {
      ModuleConstants::odrive.updateRC();
      ModuleConstants::vesc.updateRC(throttleInput);
    }

    ModuleConstants::stateMachine.runState();
    ModuleConstants::ethernet.sendTelemetry();
    lastUpdate = currentTime;
    lastWaitingPrint = currentTime;
  }

  currentTime = millis();
}
