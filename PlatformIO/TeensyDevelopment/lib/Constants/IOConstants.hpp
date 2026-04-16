#ifndef IO_CONSTANTS
#define IO_CONSTANTS

/*-----------------------------------------------------------------------------*/
/** 
 * @file   IOConstants.hpp
 * @brief  Header for IOConstants struct 
 * 
 * The IOConstants struct is used for the definition of various constants 
 * that are used for things like pin definitions and UART baudrates.
 * 
 * @author Nyx Turbeville
 * @date   March 18, 2026
*//*---------------------------------------------------------------------------*/

#include <Arduino.h>

#include <LightBeacon.hpp>


namespace Constants {
  /**
   * @brief Constats used to interface input and output 
   */
  struct IOConstants {
    static const int ledBuiltIn = 13; // Built in LED pin

    static const int oDriveRelay = 3;
    static const int eBrakeRelay = 4;
    static const int vescRelay = 5;

    static const int redLedRelay = 21;
    static const int greenLedRelay = 22;
    static const int yellowLedRelay = 23;

    static const int fanPin = 29;

    static constexpr double updateFrequency = 50.0;
    static constexpr double ledBlinkFrequency = 1.0;
    static constexpr double printSerialFrequency = 1.0;

    static const unsigned long serialBaudrate = 9'600UL;
    static const unsigned long sBusBaudrate = 100'000UL;
    static const unsigned long oDriveBaudrate = 115'200UL;
    static const unsigned long vescBaudrate = 115'200UL;

    static constexpr HardwareSerial & sBusSerial = Serial1;   // UART used for the RC receiver
    static constexpr HardwareSerial & oDriveSerial = Serial6; // UART used for the ODrive
    static constexpr HardwareSerial & vescSerial = Serial2;   // UART used for the VESC

    static const Signals::BeaconPins lightPins;

    static const Signals::ColorLED colorOff;
    static const Signals::ColorLED red;
    static const Signals::ColorLED green;
    static const Signals::ColorLED yellow;
  };

  Signals::BeaconPins lightPins {IOConstants::redLedRelay, IOConstants::greenLedRelay, IOConstants::yellowLedRelay};
  
  Signals::ColorLED colorOff {false, false, false};
  Signals::ColorLED red {true, false, false};
  Signals::ColorLED green {false, true, false};
  Signals::ColorLED yellow {false, false, true};
}

#endif // IO_CONSTANTS