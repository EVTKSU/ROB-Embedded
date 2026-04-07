#ifndef MODULE_CONSTANTS
#define MODULE_CONSTANTS

/*-----------------------------------------------------------------------------*/
/** 
 * @file   ModuleConstants.hpp
 * @brief  Header for ModuleConstants struct 
 * 
 * The ModuleConstants struct is used for the definition of various object 
 * instances that are used throughout the entire codebase 
 * 
 * @author Austin Sheppard
 * @date   March 18, 2026
*//*---------------------------------------------------------------------------*/

#include <EVT_RC.hpp>

namespace Constants {
  struct ModuleConstants {
    static Signals::ControlRC transmitter;
  };


  /* ------------------------------------------------------------ //
   * Note: 
   *  Since the values inside ModuleConstants are objects,
   *  they must be defined outside of the struct
  // ------------------------------------------------------------ */
  
  Signals::ControlRC ModuleConstants::transmitter;
}

#endif // MODULE_CONSTANTS