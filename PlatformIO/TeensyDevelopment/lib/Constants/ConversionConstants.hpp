#ifndef CONVERSION_CONSTANTS
#define CONVERSION_CONSTANTS

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
  };
}; 

#endif // CONVERSION_CONSTANTS