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

#include <EVT_VescDriver.hpp>
#include <EVT_ODriver.hpp>

#include <EVT_StateMachine.hpp>
#include <EVT_Ethernet.hpp>
#include <EVT_AutoMode.hpp>
#include <EVT_DriveEncoder.hpp>
#include <LightBeacon.hpp>
#include <EVT_RC.hpp>

#include "IOConstants.hpp"

namespace Constants {
  /**
   * @brief Modules used throughout the code base 
   */
  struct ModuleConstants {
    static Signals::StateMachine stateMachine;
    static Signals::ControlRC transmitter;
    static Signals::AutoDriver autoDriver;
    static Signals::EthernetEVT ethernet;
    static Signals::LightBeacon light;
    static ::DriveEncoder driveEncoder;
    
    static MotorControls::VescDriver vesc;    
    static MotorControls::ODriver odrive;
  };


  /* ------------------------------------------------------------ //
   * Note:
   *   Since the values inside ModuleConstants are objects,
   *   they must be defined outside of the struct 
  // ------------------------------------------------------------ */

  Signals::LightBeacon ModuleConstants::light {IOConstants::yellow, IOConstants::lightPins};
  Signals::StateMachine ModuleConstants::stateMachine;
  Signals::ControlRC ModuleConstants::transmitter;
  Signals::AutoDriver ModuleConstants::autoDriver;
  Signals::EthernetEVT ModuleConstants::ethernet;
  ::DriveEncoder ModuleConstants::driveEncoder;
  
  MotorControls::ODriver ModuleConstants::odrive;
  MotorControls::VescDriver ModuleConstants::vesc {&IOConstants::vescSerial};
}

#endif // MODULE_CONSTANTS
