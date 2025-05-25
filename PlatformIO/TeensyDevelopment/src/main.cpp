#include <Arduino.h>
#include "EVT_Ethernet.h"
#include "EVT_RC.h"
#include "EVT_VescDriver.h"
#include "EVT_ODriver.h"
#include "EVT_AutoMode.h"

void setup() {
  Serial.begin(9600);
  delay(1000); // Wait for Serial Monitor to open
  
  // Initialize modules.
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
}

void loop() {
  updateSbusData();
  // If RC data is available and channel 6 exceeds the threshold, run autonomous mode.
  if (channels[6] > 1000) {
    updateAutonomousMode();
  } else {
    updateVescControl();
    updateOdrvControl();
  }
}
