#include <Arduino.h>

#include <EVT_RC.hpp>

#include "IOConstants.hpp"
#include "TransmitterConstants.hpp"
using namespace Constants;

namespace {
  Signals::ControlRC transmitter;

  constexpr unsigned long kPrintIntervalMs = 50UL;
  unsigned long lastPrintMs = 0UL;
  unsigned long lastWaitingPrintMs = 0UL;
}


void setup() {
  Serial.begin(IOConstants::serialBaudrate);
  pinMode(IOConstants::ledBuiltIn, OUTPUT);

  while (!Serial && millis() < 4'000UL) {
  }

  Serial.println("SBUS monitor starting");
  Serial.println("USB serial uses IOConstants::serialBaudrate");
  Serial.println("SBUS input uses IOConstants::sBusSerial (Serial1 / RX pin 0)");

  lastPrintMs = millis();
  lastWaitingPrintMs = lastPrintMs;
}


void loop() {
  const bool gotValidFrame = transmitter.update();
  const unsigned long now = millis();

  if (!gotValidFrame) {
    if ((now - lastWaitingPrintMs) >= 1'000UL) {
      Serial.println("Waiting for valid SBUS frame...");
      lastWaitingPrintMs = now;
    }
    return;
  }

  digitalWrite(IOConstants::ledBuiltIn, HIGH);

  if ((now - lastPrintMs) < kPrintIntervalMs) {
    return;
  }

  uint16_t *channels = transmitter.getValueArray();
  for (int i = 0; i < 12; i++) {
    Serial.print((i + 1) < 10 ? "Ch[0" : "Ch[");
    Serial.print(i + 1);
    Serial.print("] - ");
    Serial.print(channels[i]);
    Serial.print(i < 11 ? "\t| " : "\n");
  }

  lastPrintMs = now;
  lastWaitingPrintMs = now;
}
