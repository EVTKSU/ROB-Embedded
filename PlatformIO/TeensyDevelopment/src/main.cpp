#include <Arduino.h>

#include "TransmitterConstants.hpp"
#include "ConversionConstants.hpp"
#include "ModuleConstants.hpp"
#include "IOConstants.hpp"
using namespace Constants;

unsigned long currentTime = 0UL;
unsigned long lastUpdate = 0UL;


/**
 * @brief One time setup code
 */
void setup() {
  Serial.begin(IOConstants::serialBaudrate);

  pinMode(IOConstants::ledBuiltIn, OUTPUT);

  ModuleConstants::transmitter.setMapping(TransmitterConstants::defaultJoystick, Signals::ControlRC::mapType::JOYSTICK);
  ModuleConstants::transmitter.setMapping(TransmitterConstants::defaultSwitch, Signals::ControlRC::mapType::SWITCH);
  ModuleConstants::transmitter.setMapping(TransmitterConstants::defaultTriSwitch, Signals::ControlRC::mapType::TRI_SWITCH);
  ModuleConstants::transmitter.setMapping(TransmitterConstants::defaultKnob, Signals::ControlRC::mapType::KNOB);

  // Set all the relay pins to output
  pinMode(IOConstants::oDriveRelay, OUTPUT);
  pinMode(IOConstants::eBrakeRelay, OUTPUT);
  pinMode(IOConstants::vescRelay, OUTPUT);
  pinMode(IOConstants::redLedRelay, OUTPUT);
  pinMode(IOConstants::greenLedRelay, OUTPUT);
  pinMode(IOConstants::yellowLedRelay, OUTPUT);

  // Turn on the relays to power on contactors 
  digitalWrite(IOConstants::eBrakeRelay, HIGH);

  ModuleConstants::ethernet.setupUDP();

  currentTime = millis();
}


/**
 * @brief Code to run continusously on runtime
 */
void loop() {
  // Update the RC controls when in RC
  if ((currentTime - lastUpdate) >= (ConversionConstants::secToMillis / IOConstants::updateFrequency)) {
    ModuleConstants::transmitter.update();

    Serial.println(ModuleConstants::transmitter.getChannelValue(Signals::ChannelRC::RIGHT_X));

    ModuleConstants::ethernet.sendTelemetry();
    ModuleConstants::ethernet.receiveUDP();

    lastUpdate = millis();
  }

  currentTime = millis();
}
