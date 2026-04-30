#ifndef EVT_ODRIVER_H
#define EVT_ODRIVER_H

#include <Arduino.h>
#include <ODriveUART.h>
#include <ODriveArduino.h>
#include <SoftwareSerial.h>

#include <EVT_SlewRateLimiter.hpp>

#include "ControlConstants.hpp"
#include "IOConstants.hpp"
using namespace Constants;


namespace MotorControls {
  /**
   * @brief Class used for control of an ODrive
   */
  class ODriver {
    private:
      ODriveUART oDrive {IOConstants::oDriveSerial}; // ODrive object for control 

      bool systemInitialized = false;          // Condition to denote initialization state
      bool errorClearFlag = false;
      float currentTarget = 0.0f;              // Current target position in ODrive turns

      String odrvDebug;                        // ODrive debug string
      ODriveFeedback fb;                       // ODrive feedback for position and velocity
      float cachedVoltage = 0.0f;              // Last sampled ODrive voltage
      float cachedCurrent = 0.0f;              // Last sampled ODrive current
      
      float absCenterPos = 0.0f;               // Mesaured center position in ODrive turns
      float currentPos = 0.0f;                 // Current ODrive position in ODrive turns

      const double steeringCenterTime = 10.0;  // Allowed time delay to center the steering 

      SlewRateLimiter turnLimiter {ControlConstants::oDriveSteeringInputLimit}; // Rate limiter for the steering motor
      
      size_t lastPrintTime = 0UL;              // Previous print time in miliseconds
      size_t initTime = 0UL;                   // Start time for the initialization method 

      /**
       * @brief Converts an absolute ODrive position to a steering angle.
       *
       * @param positionTurns Absolute ODrive position in turns
       * @return Steering angle in degrees relative to the calibrated center
       */
      float turnsToSteeringDegrees(float positionTurns) const;
    public:
      /**
       * @brief Defines a new ODriver instance
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
       * @tparam N Size of the array 
       * @param cmd String array of ASCII protocol commands
       */
      template <size_t N>
      void sendCommands(String (&cmd)[N]);


      /**
       * @brief Reset the odrive so it can be reinitialized
       */
      void reset();


      /**
       * @brief Block the code until the ODrive reaches a given state 
       * 
       * @param state State to wait for 
       */
      void waitUntilState(ODriveAxisState state, size_t timeout = 0);


      /**
       * @brief Gets the current target position 
       * 
       * @return The current target position in turns 
       */
      float getTarget();


      /**
       * @brief Gets the current target steering angle.
       *
       * @return Target steering angle in degrees relative to the calibrated center
       */
      float getTargetDegrees();

      
      /**
       * @brief Gets the voltage of the ODrive 
       *
       * @return Voltage reading from the ODrive  
       */
      float getVoltage();


      /**
       * @brief Gets the current of the ODrive 
       * 
       * @return Current reading from the ODrive 
       */
      float getCurrent();


      /**
       * @brief Gets the cached voltage of the ODrive without querying UART.
       *
       * @return Cached ODrive voltage.
       */
      float getCachedVoltage();


      /**
       * @brief Gets the cached current of the ODrive without querying UART.
       *
       * @return Cached ODrive current.
       */
      float getCachedCurrent();


      /**
       * @brief Checks if calibration is true
       * 
       * @return A boolean value dependent on if ODrive has been calibrated
       */
      bool isCalibrated();


      /**
       * @brief Gets the ODrive feedback values 
       *
       * @return ODriveFeedback struct of position and velocity 
       */
      ODriveFeedback getFeedback();


      /**
       * @brief Gets the cached ODrive feedback without querying UART.
       *
       * @return Cached ODriveFeedback struct of position and velocity.
       */
      ODriveFeedback getCachedFeedback();


      /**
       * @brief Gets the cached steering position.
       *
       * @return Cached steering angle in degrees relative to the calibrated center
       */
      float getCachedSteeringDegrees();


      /**
       * @brief Gets the current ODrive axis state 
       * 
       * @return ODriveAxisState The current axis state 
       */
      ODriveAxisState getAxisState();
  };
}


#endif // EVT_ODRIVER_H
