#include <Arduino.h>

#include "TransmitterConstants.hpp"
#include "ConversionConstants.hpp"
#include "ModuleConstants.hpp"
#include "IOConstants.hpp"
using namespace Constants;

size_t currentTime = 0;      // Current timestamp in milliseconds
size_t lastUpdate = 0;       // Timestamp in milliseconds of the last attempted update to all values 
size_t lastWaitingPrint = 0; // Timestamp in milliseconds of the last serial output denoting a bad frame
size_t lastPrintMessage = 0; // Timestamp in milliseconds of the last serial output
size_t lastMotorPrint = 0;   // Timestamp in milliseconds of the last motor debug output
size_t lastTelem = 0;        // Timestamp in milliseconds of the last telemetry packet

bool ledState = false; // Current state of the on board LED


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
    // Blink the LED beacon yellow with a 2.0 second period
    ModuleConstants::light.setColorState(IOConstants::yellow, true, 2.0f);

    if ((currentTime - lastUpdate) >= (ConversionConstants::secToMillis / (2 * IOConstants::ledBlinkFrequency))) {
      digitalWrite(IOConstants::ledBuiltIn, ledState = !ledState ? HIGH : LOW);
    }
  });

  // Sets the behavior of the IDLE state
  ModuleConstants::stateMachine.defineState(Signals::States::IDLE, [&] () {
    // Set the LED beacon to solid yellow
    ModuleConstants::light.setColorState(IOConstants::yellow);

    if (ModuleConstants::transmitter.getChannelValue(Signals::ChannelRC::SWF, Signals::ControlRC::mapSwitches) && !ModuleConstants::transmitter.getChannelValue(Signals::ChannelRC::SWH, Signals::ControlRC::mapSwitches)) {
      // Turn off the LED beacon and go into RC state 
      ModuleConstants::light.setColorState(IOConstants::colorOff);
      digitalWrite(IOConstants::eBrakeRelay, HIGH);
      ModuleConstants::stateMachine.setState(Signals::States::RC);
    }
  });

  // Sets the behavior of the RC state
  ModuleConstants::stateMachine.defineState(Signals::States::RC, [&] () {
    // Set the LED beacon to solid green 
    ModuleConstants::light.setColorState(IOConstants::green);

    if (ModuleConstants::transmitter.getChannelValue(Signals::ChannelRC::SWD, Signals::ControlRC::mapSwitches)) {
      // Turn off the LED beacon and go into AUTO state
      ModuleConstants::light.setColorState(IOConstants::colorOff);
      ModuleConstants::stateMachine.setState(Signals::States::AUTO);
    } else if (ModuleConstants::transmitter.getChannelValue(Signals::ChannelRC::SWH, Signals::ControlRC::mapSwitches)) {
      // Turn off the LED beacon and go into RESET state 
      ModuleConstants::light.setColorState(IOConstants::colorOff);
      ModuleConstants::stateMachine.setState(Signals::States::RESET);
    } else {
      // Update the values for the ODrive and VESC RC 
      ModuleConstants::odrive.updateRC();
      const uint16_t throttleInput = ModuleConstants::transmitter.getChannelValue(Signals::ChannelRC::LEFT_Y, false);
      const uint16_t brakeInput = ModuleConstants::transmitter.getChannelValue(Signals::ChannelRC::RIGHT_Y, false);
      if (ModuleConstants::odrive.isCalibrated()) {
        ModuleConstants::vesc.updateRC(
          throttleInput,
          brakeInput
        );
      } else {
        ModuleConstants::vesc.updateRC(
          TransmitterConstants::midRC,
          TransmitterConstants::midRC
        );
      }

      if (Serial && IOConstants::motorDataToSerial && ((currentTime - lastMotorPrint) >= 250UL)) {
        // Print the Current values the car is trying to achieve 
        Serial.printf(
          "RC steer=%u throttle=%u brake=%u\t| Steering - %.3f deg\t| ",
          ModuleConstants::transmitter.getChannelValue(Signals::ChannelRC::RIGHT_X, false),
          throttleInput,
          brakeInput,
          ModuleConstants::odrive.getTargetDegrees()
        );
        ModuleConstants::vesc.printState();
        lastMotorPrint = millis();
      }
    }
  });

  // Sets the behavior of the AUTO state
  ModuleConstants::stateMachine.defineState(Signals::States::AUTO, [&] () {
    // Blink the LED beacon green with a 2.0 second period
    ModuleConstants::light.setColorState(IOConstants::green, true, 2.0f);

    if (ModuleConstants::transmitter.getChannelValue(Signals::ChannelRC::SWH, Signals::ControlRC::mapSwitches)) {
      // Turn off the LED beacon and go into IDLE state 
      ModuleConstants::light.setColorState(IOConstants::colorOff);
      ModuleConstants::stateMachine.setState(Signals::States::IDLE);
    }

    // Updates the VESC and ODrive commands in AUTO mode 
    ModuleConstants::autoDriver.updateAuto(ModuleConstants::ethernet.receiveUDP());
  });

  // Sets the behavior of the ERROR state
  ModuleConstants::stateMachine.defineState(Signals::States::ERROR, [&] () {
    // Set the LED beacon to solid red 
    ModuleConstants::light.setColorState(IOConstants::red);

    Serial.println("An error occurred");
    Serial.println("Toggle SWH to reset");

    if (ModuleConstants::transmitter.getChannelValue(Signals::ChannelRC::SWH, Signals::ControlRC::mapSwitches)) {
      // Turn off the LED beacon and go into RESET state 
      ModuleConstants::light.setColorState(IOConstants::colorOff);
      ModuleConstants::stateMachine.setState(Signals::States::RESET);
    }

    digitalWrite(IOConstants::oDriveRelay, LOW);
    digitalWrite(IOConstants::eBrakeRelay, LOW);
    digitalWrite(IOConstants::vescRelay, LOW);
  });

  // Sets the behavior of the STOP state
  ModuleConstants::stateMachine.defineState(Signals::States::STOP, [&] () {
    // Blink the LED beacon red with a 2.0 second period
    ModuleConstants::light.setColorState(IOConstants::red, true, 2.0f);
  });

  // Sets the behavior of the RESET state
  ModuleConstants::stateMachine.defineState(Signals::States::RESET, [&] () {
    // Set the LED beacon to off
    ModuleConstants::light.setColorState(IOConstants::colorOff);
    digitalWrite(IOConstants::eBrakeRelay, LOW);

    // Reset the ODrive and clear errors 
    ModuleConstants::odrive.reset();

    Serial.println("Resetting the ODrive");
    delay(5'000);

    // Set go into IDLE state 
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
  ModuleConstants::driveEncoder.setup();
  ModuleConstants::odrive.setup();
  ModuleConstants::dynamicBrake.setup();
  ModuleConstants::ethernet.setupUDP();

  // Set the car into idle state
  ModuleConstants::stateMachine.setState(Signals::States::IDLE);
}


/**
 * @brief Code to run continusously on runtime
 */
void loop() {
  // Records the current timestamp
  currentTime = millis();

  // Limits the update frequency so that the Teensy doesn't get overloaded
  if ((currentTime - lastUpdate) >= (ConversionConstants::secToMillis / IOConstants::updateFrequency)) {
    // Update the drive encoder position accumulator before any RC-related early exit.
    ModuleConstants::driveEncoder.feed();

    if (Serial && IOConstants::telemetryToSerial && ((currentTime - lastPrintMessage) >= (ConversionConstants::secToMillis / IOConstants::printSerialFrequency))) {
      Serial.printf("Drive encoder: %.6f turns\n", ModuleConstants::driveEncoder.getPosition());
      lastPrintMessage = millis();
    }

    const bool transmitterUpdated = ModuleConstants::transmitter.update();

    if (!transmitterUpdated) {
      // Prints to the Serial Monitor about a bad frame every 5.0 seconds
      if ((currentTime - lastWaitingPrint) >= 5'000UL) {
        Serial.println("Waiting for valid SBUS frame...");
        lastWaitingPrint = millis();
      }
    } else {
      if (ModuleConstants::transmitter.getChannelValue(Signals::ChannelRC::SWH, Signals::ControlRC::mapSwitches)) {
        digitalWrite(IOConstants::eBrakeRelay, LOW);
      }

      // Runs the current state of the state machine only after a valid SBUS update.
      ModuleConstants::stateMachine.runState();
    }

    // Records the last update timestamp
    lastUpdate = millis();
  }

  // Limits the telemetry frequency so that the Teensy and ODrive don't get overloaded
  if ((currentTime - lastTelem) >= (ConversionConstants::secToMillis / IOConstants::telemetryFrequency)) {
    // Sends UDP telemetry to the Latte Panda 
    ModuleConstants::ethernet.sendTelemetry(
      ModuleConstants::stateMachine.checkError(),
      ModuleConstants::stateMachine.toString(ModuleConstants::stateMachine.getState()),
      ModuleConstants::vesc.getState().erpmCommand / ControlConstants::vescPolePairs,
      ModuleConstants::odrive.getCachedSteeringDegrees(),
      ModuleConstants::odrive.getCachedVoltage(),
      ModuleConstants::vesc.getVoltage(),
      ModuleConstants::odrive.getCachedCurrent(),
      ModuleConstants::vesc.getCurrent(),
      ModuleConstants::odrive.getTargetDegrees(),
      ModuleConstants::transmitter.getChannelValue(Signals::ChannelRC::RIGHT_X, false),
      ModuleConstants::transmitter.getChannelValue(Signals::ChannelRC::LEFT_Y, false),
      ModuleConstants::driveEncoder.getRevolutionsFromStart(),
      ModuleConstants::driveEncoder.getCount()
    );

    // Records the last telemetry timestamp
    lastTelem = millis();
  }
}
