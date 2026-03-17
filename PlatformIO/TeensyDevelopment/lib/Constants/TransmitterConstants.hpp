#ifndef TRANSMITTER_CONSTANTS
#define TRANSMITTER_CONSTANTS

/**
 * @brief Namespace used for definition of various codebase constants
 */
namespace Constants {
  /**
   * @brief Constants used for recieving and converting data from sBus
   */
  struct TransmitterConstants {
    static const int numChannels = 16;      // Number of channels received from sBus

    static constexpr uint16_t duoValues[2] = {307, 1'640};
    static constexpr uint16_t triValues[3] = {307, 974, 1'640};

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