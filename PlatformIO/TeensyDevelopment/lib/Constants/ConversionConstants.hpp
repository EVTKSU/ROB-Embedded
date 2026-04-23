#ifndef CONVERSION_CONSTANTS
#define CONVERSION_CONSTANTS

/*-----------------------------------------------------------------------------*/
/** 
 * @file   ConversionConstants.hpp
 * @brief  Header for ControlConstants struct 
 * 
 * The ControlConstants struct is used for the definition of various constants 
 * that are used for system behaviors. For example, you would find the values 
 * for PID gains or slewrates here.
 * 
 * @author Nyx Turbeville
 * @date   March 18, 2026
*//*---------------------------------------------------------------------------*/

#include <Arduino.h>

namespace Constants {
  /**
   * @brief Constants used for various conversion ratio 
   */
  struct ConversionConstants {
    static constexpr double secToMillis = 1'000.0;
    static constexpr double millisToSec = (1 / 1'000.0);

    static constexpr double degToRad = (PI / 180);
    static constexpr double radToDeg = (180 / PI);

    static constexpr double radToTurns = (1 / (2 * PI));
    static constexpr double turnsToRad = (2 * PI);

    static constexpr double degToTurns = (1 / 360);
    static constexpr double turnsToDeg = 360;
    
	  static constexpr uint8_t isDriveEncoderInverted = 0;
  };
}

#endif // CONVERSION_CONSTANTS
