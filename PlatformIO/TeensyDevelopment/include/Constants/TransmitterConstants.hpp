#ifndef EVT_TRANSMITTER_CONSTANTS
#define EVT_TRANSMITTER_CONSTANTS

namespace Constants {
  /**
   * @brief Constants used for recieving and converting data from SBUS
   */
  struct TransmitterConstants {
    /**
     * @brief RC channels used to control the car
     */
    enum Channels {
      CHANNEL1 = 0,       //
      THROTTLE,           // Channel 2
      BRAKE,              // Channel 3
      STEERING,           // Channel 4
      RESET_SWITCH,       // Channel 5
      CALIBRATION_SWITCH, // Channel 6
      AUTO_SWITCH,        // Channel 7
      CHANNEL8,
      CHANNEL9,
      CHANNEL10
    };

    /* --------------------{ Channel Transmitter Constants }------------------- */

    static const int channelDeadband = 20;        // Value for the controller deadband on the RC joysticks 
    
    static const int channelLow = 0;              // Value for accessing the low bound of an RC channel 
    static const int channelNeutral = 1;          // Value for accessing the neutral value of an RC Channel
    static const int channelHigh = 2;             // Value for accessing the high bound of an RC channel
    static constexpr int channelBounds[10][3] = { // High and low values for each channel of the RC transmitter
      {0, 0, 0},
      {0, 0, 0},
      {0, 0, 0},
      {0, 0, 0},
      {0, 0, 0},
      {0, 0, 0},
      {0, 0, 0},
      {0, 0, 0},
      {0, 0, 0},
      {0, 0, 0}
    };

    /* ------------------------------------------------------------------------ */


    /* ---------------------{ VESC Transmitter Constants }--------------------- */
    
    static const int neutralThrottle = 990;

    static const float vescForwardRange = (1700 - (neutralThrottle + channelDeadband));
    static const float vescReverseRange = ((neutralThrottle - channelDeadband) - 350);

    static const int neutralBrakeValue = 1030;
    static const int maxBrakeValue = 330;
    static const float vescBrakeRange = neutralBrakeValue - maxBrakeValue;

    /* ------------------------------------------------------------------------ */
  };
};

#endif // EV_TRANSMITTER_CONSTANTS