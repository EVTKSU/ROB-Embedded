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

bool ledState = false; // Current state of the on board LED

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
    ModuleConstants::light.setColorState(IOConstants::yellow, true, 2.0f);

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
    ModuleConstants::light.setColorState(IOConstants::green, true, 2.0f);

    if (ModuleConstants::transmitter.getChannelValue(Signals::ChannelRC::SWG, Signals::ControlRC::mapSwitches)) {
      ModuleConstants::stateMachine.setState(Signals::States::IDLE);
    } else {
      updateAutonomousMode();
    }
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
    ModuleConstants::light.setColorState(IOConstants::red, true, 2.0f);
  });

  // Sets the behavior of the RESET state
  ModuleConstants::stateMachine.defineState(Signals::States::RESET, [&] () {
    ModuleConstants::light.setColorState(IOConstants::colorOff);

    ModuleConstants::odrive.reset();

    Serial.println("Resetting the ODrive");
    delay(5'000);

    ModuleConstants::stateMachine.setState(Signals::States::IDLE);
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
  // Keep SBUS handling in the RC module
  // Wait for SWA-triggered ODrive calibration before transitioning to RC
  if ((currentTime - lastUpdate) >= (ConversionConstants::secToMillis / IOConstants::updateFrequency)) {
    if (!ModuleConstants::transmitter.update()) {
      if ((currentTime - lastWaitingPrint) >= 5'000UL) {
        Serial.println("Waiting for valid SBUS frame...");
        lastWaitingPrint = millis();
      }

      return;
    }

    throttleInput = ModuleConstants::transmitter.getChannelValue(Signals::ChannelRC::RIGHT_Y, false);

    ModuleConstants::stateMachine.runState();
    ModuleConstants::ethernet.sendTelemetry();

    lastUpdate = millis();
  }

  currentTime = millis();
}
