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
#include <EVT_Ethernet.hpp>

namespace Constants {
  struct ModuleConstants {
    static Signals::ControlRC transmitter;
    static Signals::EthernetEVT ethernet;
  };
}

#endif // MODULE_CONSTANTS
