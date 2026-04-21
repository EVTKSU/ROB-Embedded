#ifndef CONTROL_CONSTANTS
#define CONTROL_CONSTANTS

#include <stdint.h>

/*-----------------------------------------------------------------------------*/
/** 
 * @file   ControlConstants.hpp
 * @brief  Header for ControlConstants struct 
 * 
 * The ControlConstants struct is used for the definition of various constants 
 * that are used for system behaviors. For example, the values for PID gains 
 * or slewrates can be found here.
 * 
 * @author Nyx Turbeville
 * @date   March 18, 2026
*//*---------------------------------------------------------------------------*/

namespace Constants {
  /**
   * @brief Constants used for control of the system's behavior 
   * @note More or less, these are used for PID gains and rate limits
   */
  struct ControlConstants {
    static constexpr float vescMinBrake = 0.0f;
    static constexpr float vescMaxBrake = 30.0f;
    
    static constexpr float vescMinERPM = 0.0f;
    static constexpr float vescMaxERPM = 7'500.0f;
    static constexpr float vescERPMLimit = 1'500.0f;
  };
}

#endif // CONTROL_CONSTANTS
