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

#ifndef EVT_RC_H
#include <EVT_RC.hpp>
#endif 

#ifndef EVT_ODRIVER_H
#include <EVT_ODriver.hpp>
#endif 

namespace Constants {
  struct ModuleConstants {
    static Signals::ControlRC transmitter;
    static MotorControls::ODriver odrive;
  };


  /* ------------------------------------------------------------ //
   * Note:
   *   Since the values inside ModuleConstants are objects,
   *   they must be defined outside of the struct 
  // ------------------------------------------------------------ */

  Signals::ControlRC ModuleConstants::transmitter;
  MotorControls::ODriver ModuleConstants::odrive;
}

#endif // MODULE_CONSTANTS