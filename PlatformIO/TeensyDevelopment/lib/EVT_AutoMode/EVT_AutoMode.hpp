#ifndef EVT_AUTOMODE_H
#define EVT_AUTOMODE_H

/*-----------------------------------------------------------------------------*/
/** 
 * @file   EVT_AutoMode.hpp
 * @brief  Header for AutoDriver class
 * 
 * The AutoDriver class is used to receive and interpret UDP packet commands 
 * from the Latte Panda Sigma and control the VESC and ODrive accordingly.
 * 
 * @author Nyx Turbeville
 * @date   April 20, 2026
*//*---------------------------------------------------------------------------*/

#include <Arduino.h>
#include <string>

// Autonomous mode function prototype.
void updateAutonomousMode();
void setControls(const std::string &udpData);
void CtrlVesc();
void CtrlOdrive();

namespace Signals {
  /**
   * @brief Class used to receive autonomous commands and 
   */
  class AutoDriver {
    private:
      double vescRPM;          // VESC RPM setting
      double brakeCurrent;     // Brake current in amps
      double brakePercent;     // Brake current as a percentage 

      double steeringAngle;    // Steering angle in degrees

      bool emergencyFlag;      // Condition to tell if an error occured

      const int numFields = 4; // Number of expected fields in the UDP packet
      char udpBuffer[128];     // Copy of the received UDP data
      char * token;         
      int index; 
    public:
      /**
       * @brief Updates the autonomous commands given a UDP packet
       * 
       * @note The expected format is "throttle,steering,brake,emergency"
       * 
       * @param udpData UDP packet to parse and use for updates 
       */
      void updateAuto(const std::string & udpData);


      /**
       * @brief Updates the ODrive steering controls in AUTO mode 
       * 
       * @note The maximum angle the ODrive can turn from the center is 37.8 degrees
       * 
       * @param steeringAngle Steering anlge for the ODrive to move to 
       */
      void updateODrive(float steeringAngle);


      /**
       * @brief Updates the VESC throttle controls in AUTO mode 
       * 
       * @param rpm RPM value to run 
       * @param current Brake current measured in amps
       */
      void updateVESC(float rpm, float current);
  };
}

#endif // EVT_AUTOMODE_H