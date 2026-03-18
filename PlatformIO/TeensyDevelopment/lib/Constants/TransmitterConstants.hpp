#ifndef TRANSMITTER_CONSTANTS
#define TRANSMITTER_CONSTANTS

/*-----------------------------------------------------------------------------*/
/** 
 * @file   TransmitterConstants.hpp
 * @brief  Header for TransmitterConstants struct 
 * 
 * The TransmitterConstants struct is used for the definition of various constants 
 * that are used for all values related to RC control. These values include things
 * like the number of channels on the RC transmitter, defult mapping arrays,
 * expected RC minimum, medium, and maximum values.
 * 
 * @author Nyx Turbeville
 * @date   March 18, 2026
*//*---------------------------------------------------------------------------*/

/**
 * @brief Namespace used for definition of various codebase constants
 */
namespace Constants {
  /**
   * @brief Constants used for recieving and converting data from sBus
   */
  struct TransmitterConstants {
    static const int numChannels = 16;      // Number of channels received from sBus

    static constexpr uint16_t defaultJoystick[2] = {0, 100};
    static constexpr uint16_t defaultSwitch[2] = {0, 100};
    static constexpr uint16_t defaultTriSwitch[3] = {0, 50, 100};
    static constexpr uint16_t defaultKnob[2] = {0, 100};

    static const uint16_t minRC = 307;      // Minimum RC value
    static const uint16_t midRC = 974;      // Mid range RC value
    static const uint16_t maxRC = 1'640;    // Maximum RC value
  };
}

#endif // TRANSMITTER_CONSTANTS