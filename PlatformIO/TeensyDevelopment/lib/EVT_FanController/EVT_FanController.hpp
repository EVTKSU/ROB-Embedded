#ifndef FAN_CONTROLLER
#define FAN_CONTROLLER

#include <Arduino.h>
#include <Adafruit_MCP9808.h>

#include <IOConstants.hpp>
using Constants::IOConstants;

/*-----------------------------------------------------------------------------*/
/** @file    FanController.hpp
  * @brief   Header for FanController class (used to get temp data and set fan speed)
*//*---------------------------------------------------------------------------*/


/**
 * @brief Units of temperature to recieve from the senors
 */
enum temperatureUnits {
  celcius = 0,
  fahrenheit
};


/**
 * @brief Class used to control and recieve data from a fan control module
 */
class FanController {
  private:
    const int fanPin = IOConstants::fanPWM;       // Fan controller PWM input

    Adafruit_MCP9808 tempSensor;                  // Fan controller I2C temperature sensor 

    float tempCelcius;                            // Current temperature in celcius
    float tempFahrenheit;                         // Current temperature in farenheit 

    int pwmOutput;                                // Fan PWM Output
    const double pwmScale = 1;                    // Temperature Scale for PWM value
    static const int maxPWM = 255;                // Maximum value for analogWrite

    static constexpr double minSpeedScale = 0.20; // Minimum speed percentage for the fan
    static const int minPWM = maxPWM * minSpeedScale;

  public:
    /**
     * @brief Define a new fan controller 
     */
    FanController();


    /**
     * @brief Get the data from the fan module sensors 
     */
    void updateSensorData();


    /**
     * @brief Get the current temperature from the I2C sensor 
     * 
     * @param units Temperature units to use (Defualt fahrenheit)
     * @return The current temperature in the specified units 
     */
    float getTemperature(temperatureUnits units = temperatureUnits::fahrenheit);


    /**
     * @brief Set the PWM value of the fan 
     * 
     * @note This method uses digitalWrite rather than duty cycle, so the allowed values are [0, 255]
     * 
     * @param speed Speed of the FAN as a PWM value 
     */
    void setFanSpeed(int speed);


    /**
     * @brief Sets the fan speed via a proportional control scheme
     * 
     * @note Using this method, the fan will always spin at at least 25% of its maximum speed
     */
    void propTempControl();


    /**
     * @brief Sets the fan speed using a predefined value table
     * 
     * @note Using this method, the fan will always spin at at least 25% of its maximum speed
     */
    void setAutoSpeed();


    /**
     * @brief Get the PWM value for the Fan
     * 
     * @note The value is the analogWrite resolution, [0, 255], not duty cycle
     * 
     * @return The PWM value to the fan module
     */
    float getFanPWMValue();
};

#endif // FAN_CONTROLLER