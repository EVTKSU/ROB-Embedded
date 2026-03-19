#include <EVT_FanController.hpp>

namespace Modules {
  FanController::FanController() {
    if (!Serial) {             // Start the serial monitor
      Serial.begin(9'600);           
    }

    pinMode(fanPin, OUTPUT);   // Set fan output pin
  
    if (!tempSensor.begin()) { // Try to initialize the fan module
      Serial.println("Failed to find temp sensor");
    }

    tempSensor.wake();
  }


  void FanController::updateSensorData() {
    tempCelcius = tempSensor.readTempC();    // Read the temp in Celcius
    tempFahrenheit = tempSensor.readTempF(); // Read the temp in Fahrenheit
  }


  float FanController::getTemperature(temperatureUnits units) {
    switch (units) {
      case (temperatureUnits::celcius):
        return tempCelcius;
      case (temperatureUnits::fahrenheit):
        return tempFahrenheit;
      default:
        return tempCelcius;
    }
  }


  void FanController::setFanSpeed(int speed) {
    pwmOutput = speed;
    analogWrite(fanPin, pwmOutput);
  }


  void FanController::propTempControl() {
    pwmOutput = (pwmScale * tempFahrenheit) < (minPWM) ? minPWM : (pwmScale * tempFahrenheit);
    setFanSpeed(pwmOutput);
  }


  void FanController::setAutoSpeed() {
    if (tempFahrenheit >= 105) {
      pwmOutput = floor(maxPWM);
    } else if (tempFahrenheit >= 95 && tempFahrenheit < 105) {
      pwmOutput = floor(maxPWM * 0.75);
    } else if (tempFahrenheit >= 80 && tempFahrenheit < 95) {
      pwmOutput = floor(maxPWM * 0.50);
    } else {
      pwmOutput = floor(maxPWM * 0.25);
    }

    setFanSpeed(pwmOutput);
  }


  float FanController::getFanPWMValue() {
    return pwmOutput;
  }


  double timingFunc() {
    return millis() / 1'000;
  }
}