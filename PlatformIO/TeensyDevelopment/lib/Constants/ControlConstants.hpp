#ifndef CONTROL_CONSTANTS
#define CONTROL_CONSTANTS

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
   */
  struct ControlConstants {
    static constexpr float vescMinBrake = 0.0f;
    static constexpr float vescMaxBrake = 30.0f;
    
    static constexpr float vescMinERPM = 0.0f;
    static constexpr float vescMaxERPM = 7'500.0f;
    static constexpr float vescERPMLimit = 1'500.0f;

    static constexpr int vescPolePairs = 4;

    static constexpr float oDriveMaxTurns = 2.1f;
    static constexpr float oDriveVelLimit = 140.0f;
    static constexpr float oDriveAccelLimit = 400.0f;

    static constexpr float oDrivePosGain = 100.0f;           
    static constexpr float oDriveVelGain = 0.05f;            
    static constexpr float oDriveIntegratorGain = 0.0f;      

    static constexpr float oDriveSoftCurrentMax = 30.0f; // Current in amps for standard operation
    static constexpr float oDriveHardCurrentMax = 80.0f; // Current in amps for an ODrive overcurrent error

    static constexpr float oDriveMechanicalThreshold = -100;
    static constexpr float oDriveElectricalThreshold = 100;

    static constexpr float oDriveSteeringInputLimit = 50.0f; // Steering rate limit for the ODrive input

    static constexpr float oDriveGearRatio = (1.0 / 20.0);
  };
}

#endif // CONTROL_CONSTANTS