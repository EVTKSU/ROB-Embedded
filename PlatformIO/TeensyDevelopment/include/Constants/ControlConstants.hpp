#ifndef EVT_CONTROL_CONSTANTS
#define EVT_CONTROL_CONSTANTS

namespace Constants {
  /**
   * @brief Constants used for control of the car's behavior 
   * @note More or less, these are used for PID gains and rate limits
   */
  struct ControlConstants {
    static const float fanKP = 1; // Proportional gain for the PWM fan on MCB

    static const float steeringVelocityLimit = 30; // Velocoty limit for the steering motor in turns per second
    static const float steeringAccelLimit = 10;    // Acceleration limit for the steering motor in turns per second squared
    static const float steeringMaxTurns = 2.75;    // Maxmium turns the steering motor can make

    static const float vescMaxRPM = 7500;          // Maximum speed for the VESC controllers in RPM
    static const float vescMaxBrakeCurrent = 30;   // Maximum brake current for the VESC controllers in amps
    static const float vescMaxAccel = 500;         // Maximum change in RPM per second for the VESC controllers
  };
};  

#endif // EVT_CONTROL_CONSTANTS