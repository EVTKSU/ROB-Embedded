#ifndef EVT_ODRIVER_H
#define EVT_ODRIVER_H

#include <Arduino.h>
#include <ODriveUART.h>
#include <ODriveArduino.h>
#include <SoftwareSerial.h>

#include <EVT_SlewRateLimiter.hpp>

#include "IOConstants.hpp"
using namespace Constants;

// #define STATUS_LED_PIN 13

// // Global ODrive flag and debug string.

// extern HardwareSerial &odrive_serial;
// extern bool systemInitialized;
// extern String odrvDebug;
// extern float target;
// // Declare the ODriveUART object so it can be used across modules.
// extern ODriveUART odrive;

// // global ODrive function prototypes.
// void setupOdrv();
// void updateOdrvControl();
// void getOdrvErrors();
// float getTarget();


/**
 * TODO:
 *   - Confirm the mapping works 
 *   - Confirm the slewrate limiter works
**/

namespace MotorControls {
  /**
   * @brief Class used for control of an ODrive
   */
  class ODriver {
    private:
      ODriveUART oDrive {IOConstants::oDriveSerial}; // ODrive object for control 

      bool systemInitialized = false;          // Condition to denote initialization state
      bool errorClearFlag = false;
      float currentTarget;                     // Current target position 

      String odrvDebug;                        // ODrive debug string
      ODriveFeedback fb;                       // ODrive feedback for position and velocity
      
      float absCenterPos;                      // Mesaured center position 
      float currentPos;                        // Current ODrive position
      
      const float velLimit = 140.0f;           // Maximum velocity in turns per second
      const float accelLimit = 400.0f;         // Maximum acceleration in turns per second squared
      const float maxTurns = 2.1f;             // Maximum turns in either direction 

      const float positionGain = 100.0f;       // ODrive position proportional gain
      const float velocityGain = 0.05f;       // ODrive velocity proportional gain
      const float integratorGain = 0.0f;       // ODrive velocity integration gain

      const float softMaxCurrent = 30.0f;      // ODrive continuous current limit
      const float hardMaxCurrent = 80.0f;      // ODrive error current limit 

      const double steeringCenterTime = 10.0;  // Allowed time delay to center the steering 

      const float rateLimit = 50.0f;           // Maximum allowed change in position 
      SlewRateLimiter turnLimiter {rateLimit}; // Rate limiter for the steering motor
      
      unsigned long lastPrintTime = 0UL;       // Previous print time in miliseconds
      unsigned long initTime = 0UL;            // Start time for the initialization method 

      float turnsToSteeringDegrees(float turns);
    public:
      /**
       * @brief Defines a new ODriver given a serial port
       */
      ODriver();


      /**
       * @brief Checks if ODrive serial communication has been defined
       * 
       * @return ODrive serial communication state as a boolean
       */
      bool setup();


      /**
       * @brief Configures the ODrive zero position
       * 
       * @param pos Zero position 
       */
      void configureAbsoluteReference(float pos);


      
      /**
       * @brief Configures the trapezoidal trajectory limits 
       */
      void configureTrapTrajLimits();


      /**
       * @brief Gets the active errors from the ODrive as an enum
       * 
       * @return The current ODrive errors 
       */
      ODriveError getActiveErrors();


      /**
       * @brief Gets the disarm reason from the ODrive as a long
       * 
       * @return The disarm reason as a long
       */
      long getDisarmReason();


      /**
       * @brief Gets the current input mode
       * 
       * @return The current ODrive input mdoe  
       */
      ODriveInputMode getInputMode();


      /**
       * @brief Gets the current input mode as a string
       * 
       * @return The current ODrive input mode as a string
       */
      String getInputModeString();


      /**
       * @brief Runs the ODrive calibration sequence
       */
      void initCalibration();


      /**
       * @brief Control loop used to control the ODrive during RC
       */
      void updateRC();


      /**
       * @brief Control loop used to control the ODrive during auto
       */
      void updateAuto(float steering);


      /**
       * @brief Sends a command to ODrive over UART
       * 
       * @param cmd ASCII Protocol Command
       */
      void sendCommand(String cmd);


      /**
       * @brief Sends a series of commands to the ODrive over UART
       * 
       * @param cmd String array of ASCII protocol commands
       */
      void sendCommands(String cmd[]);


      /**
       * @brief Reset the odrive so it can be reinitialized
       */
      void reset();


      /**
       * @brief Gets the current target position 
       * 
       * @return The current target position in turns 
       */
      float getTarget();


      /**
       * @brief Gets the current steering target in degrees
       *
       * @return Steering target in degrees
       */
      float getTargetDegrees();


      /**
       * @brief Gets the latest ODrive feedback
       *
       * @return ODriveFeedback containing position and velocity
       */
      ODriveFeedback getFeedback();


      /**
       * @brief Gets the current steering position in degrees
       *
       * @return Steering position in degrees
       */
      float getSteeringDegrees();


      /**
       * @brief Gets the ODrive bus voltage
       *
       * @return Voltage in volts
       */
      float getVoltage();


      /**
       * @brief Gets the ODrive bus current
       *
       * @return Current in amps
       */
      float getCurrent();


      /**
       * @brief Checks if calibration is true
       * 
       * @return A boolean value dependent on if ODrive has been calibrated
       */
      bool isCalibrated();
  };
}


#endif // EVT_ODRIVER_H
