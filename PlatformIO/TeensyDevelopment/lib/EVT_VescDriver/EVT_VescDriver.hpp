#ifndef EVT_VESCDRIVER_H
#define EVT_VESCDRIVER_H

/*----------------------------------------------------------------------------------*/
/** 
 * @file   EVT_VescDriver.hpp
 * @brief  Header for Vesc driver 
 * 
 * The vesc driver is used to use external values and use those values to either 
 * increase the rpms or the break current being pushed by the vesc.
 * 
 * @author Noah Gebremariam
 * @date   March 20, 2026
*//*--------------------------------------------------------------------------------*/

#include <Arduino.h>
#include <VescUart.h>
#include <SoftwareSerial.h>

#include <EVT_SlewRateLimiter.hpp>

#include "ControlConstants.hpp"
using namespace Constants;


namespace MotorControls {
  /**
   * @brief struct used to store values from the teensy that change the output of the vesc
   */
  struct VescValues {         
    float erpmCommand = 0.0f;  // ERPM input
    float brakeCommand = 0.0f; // Brake current
  };


  /**
   * @brief Class used to control VESC operation
   */
  class VescDriver {
    private:
      VescUart vesc;            // VESC UART instance

      uint16_t throttleChannel; // Value from the RC channel used for the VESC commands

      VescValues targetValues {0, 0}; // Output values to VESC

      SlewRateLimiter rpmLimit {ControlConstants::vescERPMLimit};

      String vescDebug;         // VESC debug message as a string 
      String vescErrorString;   // VESC error message as a string
    public:
      /**
       * @brief Initializes UART communication between teensy and VESC
       * 
       * @param vescSerial A pointer to the UART port on the teensy
       */
      VescDriver(HardwareSerial * vescSerial);  


      /**
       * @brief Controls the outputs of the VESC based on RC values
       * 
       * @param throttleChannel Channel value from the RC channel 
       */
      void updateRC(uint16_t throttleChannel);


      /**
       * @brief Controls the outputs of the VESC based on values from LattePanda
       * 
       * @param erpm ERPM setpoint
       * @param brake Brake current
       */
      void updateAuto(float erpm, float brake);


      /**
       * @brief Transcribes errors from the VESC to Strings and prints them
       * 
       * @param VESC_ERR Error signal from VESC as an enum
       */
      String errorToString(mc_fault_code VESC_ERR);


      /**
       * @brief Prints the current target values (ERPM and Brake current) for the VESC
       * 
       */
      void printState();

      
      /**
       * @brief Gets the current target values (ERPM and Brake current) for the VESC
       * 
       * @return VescValues 
       */
      VescValues getState();


      /**
       * @brief Gets the VESC input voltage
       * 
       * @return VESC input voltage as a float 
       */
      float getVoltage();


      /**
       * @brief Gets the VESC current
       * 
       * @return VESC input current as a float 
       */
      float getCurrent();
  };
}


#endif // EVT_VESCDRIVER_H
