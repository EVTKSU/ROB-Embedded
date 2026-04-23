#include <Arduino.h>

#include <EVT_AutoMode.hpp>
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
unsigned long lastTelem = 0UL;        // Timestamp in milliseconds of the last telemetry packet

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
    // Set the LED beacon to solid yellow
    ModuleConstants::light.setColorState(IOConstants::yellow);

    if ((currentTime - lastUpdate) >= (ConversionConstants::secToMillis / (2 * IOConstants::ledBlinkFrequency))) {
      digitalWrite(IOConstants::ledBuiltIn, ledState = !ledState ? HIGH : LOW);
    }
  });

  // Sets the behavior of the IDLE state
  ModuleConstants::stateMachine.defineState(Signals::States::IDLE, [&] () {
    // Blink the LED beacon yellow with a 2.0 second period
    ModuleConstants::light.setColorState(IOConstants::yellow, true, 2.0f);

    if (ModuleConstants::transmitter.getChannelValue(Signals::ChannelRC::SWF, Signals::ControlRC::mapSwitches) && !ModuleConstants::transmitter.getChannelValue(Signals::ChannelRC::SWH, Signals::ControlRC::mapSwitches)) {
      // Turn off the LED beacon and go into RC state 
      ModuleConstants::light.setColorState(IOConstants::colorOff);
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
      // Print a message to the Serial Monitor if ODrive hasn't been initialized 
      if (!ModuleConstants::odrive.isCalibrated()) {
        Serial.println("ODrive not yet calibrated");
      }

      // Update the values for the ODrive and VESC RC 
      ModuleConstants::odrive.updateRC();
      ModuleConstants::vesc.updateRC(ModuleConstants::transmitter.getChannelValue(Signals::ChannelRC::LEFT_Y, false));

      if (IOConstants::motorDataToSerial) {
        // Print the Current values the car is trying to achieve 
        Serial.printf("Steering - %.3f\t| ", ModuleConstants::odrive.getTarget());
        ModuleConstants::vesc.printState();
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
  // Limits the update frequency so that the Teensy doesn't get overloaded
  if ((currentTime - lastUpdate) >= (ConversionConstants::secToMillis / IOConstants::updateFrequency)) {
    // Skips the loop if the SBUS gets a bad frame 
    if (!ModuleConstants::transmitter.update()) {
      // Prints to the Serial Monitor about a bad frame every 5.0 seconds
      if ((currentTime - lastWaitingPrint) >= 5'000UL) {
        Serial.println("Waiting for valid SBUS frame...");
        lastWaitingPrint = millis();
      }

      return;
    }

    // Runs the current state of the state machine 
    ModuleConstants::stateMachine.runState();

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
      ModuleConstants::odrive.getFeedback().pos,
      ModuleConstants::odrive.getVoltage(),
      ModuleConstants::vesc.getVoltage(),
      ModuleConstants::odrive.getCurrent(),
      ModuleConstants::vesc.getCurrent(),
      ModuleConstants::odrive.getTarget(),
      ModuleConstants::transmitter.getChannelValue(Signals::ChannelRC::RIGHT_X, false),
      ModuleConstants::transmitter.getChannelValue(Signals::ChannelRC::LEFT_Y, false)
    );

    // Records the last telemetry timestamp
    lastTelem = millis();
  }

  // Records the current timestamp
  currentTime = millis();
}
