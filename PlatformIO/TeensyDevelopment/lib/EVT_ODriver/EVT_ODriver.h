#ifndef EVT_ODRIVER_H
#define EVT_ODRIVER_H

#include <Arduino.h>
#include <ODriveUART.h>
#include <ODriveArduino.h>
#include <SoftwareSerial.h>

#define STATUS_LED_PIN 13

// Global ODrive flag and debug string.
extern HardwareSerial &odrive_serial; // Serial bus for communication to the ODrive
extern bool systemInitialized;        // Condition to denote whether the system has been initialized
extern String odrvDebug;              // Debug string received from the ODrive
extern float target;                  // Target steering angle for the ODrive


// Declare the ODriveUART object so it can be used across modules.
extern ODriveUART odrive; // ODriveUART object used to send commands to the ODrive 


/**
 * @brief Begin serial communication between the Teensy 4.1 and ODrive 
 * 
 */
void setupOdrv();


/**
 * @brief Update the values sent to control the ODrive 
 * 
 */
void updateOdrvControl();


/**
 * @brief Get the current target steering value from the ODrive 
 * 
 * @return The current target position as a flot  
 */
float getTarget();


#endif // EVT_ODRIVER_H