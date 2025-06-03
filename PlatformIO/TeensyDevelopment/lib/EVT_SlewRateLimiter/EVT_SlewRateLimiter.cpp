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
  currentTime = millis();              // Get the current time
  timeChange = currentTime - lastTime; // Measure the change in time 
  delta = targetValue - lastValue;

  if (delta > 0) { // Code to run to calculate change if the difference is positive
    maxDelta = maxIncrease * (timeChange / 1000.0); // Get the maximum positive change per second
    delta = min(delta, maxDelta);
  } else if (delta < 0) { // Code to run to calculate change if the difference is negative
    maxDelta = maxDecrease * (timeChange / 1000.0); // Get the maximum negative change per second
    delta = max(delta, -maxDelta);
  }

  lastValue += delta;  // Add the change to the value 
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