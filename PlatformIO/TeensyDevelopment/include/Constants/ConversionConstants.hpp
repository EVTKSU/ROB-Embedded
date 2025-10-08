#ifndef EVT_CONVERSION_CONSTANTS
#define EVT_CONVERSION_CONSTANTS

#include <Arduino.h>

namespace Constants {
  /**
   * @brief Constants used for unit conversions 
   */
  struct ConversionConstants {
    static const float millisToSec = (1 / 1000);

    static const float radToDeg = 180 / M_PI;
    static const float degToRag = M_PI / 180;
  };
};

#endif // EVT_CONVERSION_CONSTANTS