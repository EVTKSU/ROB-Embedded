#ifndef EVT_IO_CONSTANTS
#define EVT_IO_CONSTANTS

namespace Constants {
  /**
   * @brief Constants used for input and output values 
   * 
   */
  struct IOConstants {
    static const int ODriveRelayPin = 3;    // Pin for the relay that switches power for the ODrive contactor 
    static const int VESCRelayPin = 4;      // Pin for the relay that switches power for the VESC contactor 
    static const int ContactorRelayPin = 5; // Pin for the relay that switches power for the aux battery

    static const int pandaBootPin = 0;      // Pin for the digital output to boot the Latte Panda Sigma 
    static const int onBoardLED = 13;       // Pin for the Teensy's on-board LED
    static const int fanPin = 29;           // Pin for the fan PWM control 

  };
};

#endif // EVT_IO_CONSTANTS