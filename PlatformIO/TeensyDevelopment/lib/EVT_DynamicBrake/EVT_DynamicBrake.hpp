#ifndef EVT_DYNAMICBRAKE_H
#define EVT_DYNAMICBRAKE_H

/*-----------------------------------------------------------------------------*/
/**
 * @file   EVT_DynamicBrake.hpp
 * @brief  Header for dynamic brake stepper driver
 *
 * The DynamicBrake class controls a step/direction actuator used to press
 * the hydraulic brake pedal. It handles homing against a limit switch and
 * maps a normalized brake command to step position.
 *
 * @date   April 28, 2026
*//*---------------------------------------------------------------------------*/

#include <Arduino.h>


namespace MotorControls {
  /**
   * @brief Class used to control the dynamic brake linear actuator
   */
  class DynamicBrake {
    private:
      bool homed = false;
      int currentPositionSteps = 0;

      void pulseStep();
      void stepMotor(int steps, bool directionForward);
      bool isLimitHit() const;
    public:
      /**
       * @brief Sets up the stepper and limit switch pins
       */
      void setup();


      /**
       * @brief Homes the actuator backward, then moves forward to pedal offset
       *
       * @return true when the limit switch is found, false on timeout
       */
      bool home();


      /**
       * @brief Moves the actuator to a normalized brake position
       *
       * @param brakePercent Brake command in the range 0.0 to 1.0
       * @return true when the target position is reached
       */
      bool update(float brakePercent);
  };
}

#endif // EVT_DYNAMICBRAKE_H
