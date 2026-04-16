#ifndef MODULE_CONSTANTS
#define MODULE_CONSTANTS

/*-----------------------------------------------------------------------------*/
/** 
 * @file   ModuleConstants.hpp
 * @brief  Header for ModuleConstants struct 
 * 
 * The ModuleConstants struct is used for the definition of various constants 
 * that are used throughout the entire codebase for module drivers
 * 
 * @author Austin Sheppard
 * @date   March 18, 2026
*//*---------------------------------------------------------------------------*/

#include <EVT_RC.hpp>
#include <EVT_VescDriver.hpp>

#include <EVT_Ethernet.hpp>
#include <EVT_ODriver.hpp>

#include "IOConstants.hpp"

namespace Constants {
  struct ModuleConstants {
    static Signals::ControlRC transmitter;
    static Signals::EthernetEVT ethernet;
    
    static MotorControls::VescDriver vesc;    
    static MotorControls::ODriver odrive;
  };


  /* ------------------------------------------------------------ //
   * Note:
   *   Since the values inside ModuleConstants are objects,
   *   they must be defined outside of the struct 
  // ------------------------------------------------------------ */

  Signals::ControlRC ModuleConstants::transmitter;
  Signals::EthernetEVT ModuleConstants::ethernet;
  
  MotorControls::ODriver ModuleConstants::odrive;
  MotorControls::VescDriver ModuleConstants::vesc {&IOConstants::vescSerial};
}

#endif // MODULE_CONSTANTS