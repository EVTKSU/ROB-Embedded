#ifndef MODULE_CONSTANTS
#define MODULE_CONSTANTS

/*-----------------------------------------------------------------------------*/
/** 
 * @file   ModuleConstants.hpp
 * @brief  Header for ModuleConstants struct 
 * 
 * The ModuleConstants struct is used for the definition of various constants 
 * that are used throughout the entire codebase for...
 * 
 * @author Austin Sheppard
 * @date   March 18, 2026
*//*---------------------------------------------------------------------------*/

#include <EVT_RC.hpp>
#include <EVT_VescDriver.hpp>

#include "IOConstants.hpp"

namespace Constants {
  struct ModuleConstants {
    static Signals::ControlRC transmitter;
    static MotorControls::VescDriver vesc;

  };

  Signals::ControlRC ModuleConstants::transmitter;
  MotorControls::VescDriver ModuleConstants::vesc {&IOConstants::vescSerial};
}

#endif // MODULE_CONSTANTS