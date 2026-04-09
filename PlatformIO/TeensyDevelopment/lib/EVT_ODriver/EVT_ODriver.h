#ifndef EVT_ODRIVER_H
#define EVT_ODRIVER_H

#include <Arduino.h>
#include <ODriveUART.h>
#include <ODriveArduino.h>
#include <SoftwareSerial.h>
#include "IOConstants.hpp"

constexpr int STATUS_LED_PIN = Constants::IOConstants::ledBuiltIn;

// Global ODrive flag and debug string.

extern HardwareSerial &odrive_serial;
extern bool systemInitialized;
extern String odrvDebug;
extern float target;
// Declare the ODriveUART object so it can be used across modules.
extern ODriveUART odrive;

// global ODrive function prototypes.
void setupOdrv();
void updateOdrvControl();
void getOdrvErrors();
float getTarget();


#endif // EVT_ODRIVER_H
