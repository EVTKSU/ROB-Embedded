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
      double vescERPM;         // VESC ERPM setting
      double brakeCurrent;     // Brake current in amps

      double steeringAngle;    // Steering angle in degrees
      double requestedBrakePercent; // Brake command requested from UDP packet, 0.0 to 100.0

      bool emergencyFlag;      // Condition to tell if an error occured
      bool holdStateActive;    // Neutral command when remote state requests hold/manual
      bool launchBoostActive = false; // AUTO start assist is currently commanding current
      bool wasForwardCommandActive = false; // Previous AUTO command requested forward motion
      unsigned long launchBoostStartTime = 0;
      std::string commandState;// State text from UDP packet when present

      const int numFields = 5; // Packet: "erpm,steering_degrees,emergency,state,brake_percent"
      char udpBuffer[128];     // Copy of the received UDP data
      char * token;         
      int index; 
    public:
      /**
       * @brief Updates the autonomous commands given a UDP packet
       * 
       * @note Expected format is "erpm,steering_degrees,emergency,state,brake_percent"
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
       * @param erpm ERPM value to run 
       * @param current Brake current measured in amps
       */
      void updateVESC(float erpm, float current);


      /**
       * @brief Updates the VESC with direct drive current for AUTO launch assist.
       *
       * @param erpm ERPM setpoint to keep for telemetry
       */
      void updateVESCLaunchBoost(float erpm);
  };
}

#endif // EVT_AUTOMODE_H
