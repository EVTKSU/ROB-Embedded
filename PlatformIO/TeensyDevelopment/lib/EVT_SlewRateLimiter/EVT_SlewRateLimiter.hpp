#ifndef EVT_SLEWRATE_LIMITER
#define EVT_SLEWRATE_LIMITER

/*----------------------------------------------------------------------------------*/
/** 
 * @file   SlewRateLimiter.hpp
 * @brief  Header for SlewRateLimiter class 
 * 
 * The SlewRateLimiter class is used to allow for a simplistic limiting of the 
 * amount that values can change per second. Most often, this is used as an 
 * acceleration or velocity limiter for motor control. 
 * 
 * @author Nyx Turbeville
 * @date   March 18, 2026
*//*--------------------------------------------------------------------------------*/
 
#include <Arduino.h>


namespace MotorControls {
  /**
   * @brief Class used to limit the maxiumum amount a numerical value change per second 
   */
  class SlewRateLimiter {
    private:
      float maxIncrease; // Maximum positive change in the input per second
      float maxDecrease; // Maximum negative change in the input per second

      float currentTime; // Current time value in milliseconds 
      float lastTime;    // Time of the previous iteration
      float timeChange;  // Change in time since previous iteration

      float lastValue;   // Value of the number at the previous iteration

    public:
      /**
       * @brief Define a new SlewRateLimiter given the maximum change in either direction 
       * 
       * @param maxChange Maximum change per second
       * @param initialVal Initial value of the variable
       */
      SlewRateLimiter(float maxChange, float initialVal = 0);


      /**
       * @brief Define a new SlewRateLimiter given the maximum change in positive and negative directions
       * 
       * @param maxPosChange Maximum positive change per second
       * @param maxNegChange Maximum negative change per second
       * @param initialVal Initial value of the variable
       */
      SlewRateLimiter(float maxPosChange, float maxNegChange, float initialVal = 0);


      /**
       * @brief Calculates the allowed change in value
       * 
       * @param targetValue Target value to reach
       * @return The new value with the allowed amount of change 
       */
      float calculate(float targetValue);


      /**
       * @brief Sets the maximum rate of change
       * 
       * @param rate Maximum amount the value can change by
       */
      void setRate(float rate);

      /**
       * @brief Sets the maximum rate of change
       * 
       * @param pos Maximum positive change
       * @param neg Maximum negative change
       */
      void setRate(float pos, float neg);


      /**
       * @brief Sets the last value of the rate limiter
       * 
       * @param val Last value to set
       */
      void setLastValue(float val);
  };
}

#endif // EVT_SLEWRATE_LIMITER