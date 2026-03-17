#include "EVT_SlewRateLimiter.hpp"


SlewRateLimiter::SlewRateLimiter(float maxChange, float initialVal) {
  maxIncrease = maxDecrease = maxChange;

  lastValue = initialVal;

  currentTime = millis();
  lastTime = millis();
}


SlewRateLimiter::SlewRateLimiter(float maxPosChange, float maxNegChange, float initialVal) {
  maxIncrease = maxPosChange;
  maxDecrease = maxNegChange;

  lastValue = initialVal;

  currentTime = millis();
  lastTime = millis();
}


float SlewRateLimiter::calculate(float targetValue) {
  currentTime = millis();               // Get the current time
  timeChange = currentTime - lastTime;  // Measure the change in time
  
  lastValue += constrain(               // Constrain the value change
    targetValue - lastValue,            // Change in value since previous iteration 
    maxDecrease * (timeChange / 1000),  // Maximum decrease value 
    maxIncrease * (timeChange / 1000)   // Maximum increase value
  );

  lastTime = millis(); // Record the time at the end of the current iteration

  return lastValue;
}


void SlewRateLimiter::setRate(float rate) {
  maxIncrease = maxDecrease = rate;
}


void SlewRateLimiter::setRate(float pos, float neg) {
  maxIncrease = pos;
  maxDecrease = neg;
}


void SlewRateLimiter::setLastValue(float val) {
  lastValue = val;
}