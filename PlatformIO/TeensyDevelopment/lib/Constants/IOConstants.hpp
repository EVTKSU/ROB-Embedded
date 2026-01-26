#ifndef IO_CONSTANTS
#define IO_CONSTANTS

namespace Constants {
  /**
   * @brief Constats used to interface input and output 
   */
  struct IOConstants {
    static const int ledBuiltIn = 13; // Built in LED pin

    static const int odriveRelay = 3;
    static const int eBrakeRelay = 4;
    static const int vescRelay = 5;

    static const int redLedRelay = 21;
    static const int greenLedRelay = 22;
    static const int yellowLedRelay = 23;

    static const int fanPWM = 29;

    static const unsigned long serialBaudrate = 9600UL;
  };
}

#endif // IO_CONSTANTS