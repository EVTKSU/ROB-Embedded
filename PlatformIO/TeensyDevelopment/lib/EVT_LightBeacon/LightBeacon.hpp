#ifndef LIGHT_BEACON
#define LIGHT_BEACON

/*-----------------------------------------------------------------------------*/
/** 
 * @file   LightBeacon.hpp
 * @brief  Header for LightBeacon class
 * 
 * The light beacon class is used for control of a three pin, RGY LED dome.
 * Using the light beacon, you can set color and blink delay of the light. 
 * 
 * @author Nyx Turbeville
 * @date   April 9, 2026
*//*---------------------------------------------------------------------------*/

#include <Arduino.h>
#include <vector>

namespace Signals {
  /**
   * @brief Struct used to set LED color relays
   */
  struct ColorLED {
    bool red, green, yellow;
  };


  /**
   * @brief Struct used to set the relay pin values
   */
  struct BeaconPins {
    int red, green, yellow;
  };


  /**
   * @brief Class used for control of the light beacon
   */
  class LightBeacon {
    private:
      ColorLED currentColor;  // Current color of the LED 
      BeaconPins colorBeacon; // Pins for the relays for the lights 

      bool isBlinking;        // Blink condition for the current state 
      bool isOn;              // Condition for if the beacon is on 
      int blinkDelay;         // Blink delay period

      size_t lastBlink = 0UL; 
    public:
      /**
       * @brief Define an instance of the LED beacon 
       * 
       * @param initColor Initial color
       * @param pins A BeaconPin struct for the setting of the relay pins 
       */
      LightBeacon(ColorLED initColor, BeaconPins pins);


      /**
       * @brief Sets the color and blink state of the light beacon
       * 
       * @param color Current color of the LED beacon
       * @param blink Condition for if the LED is blinking or not (Default off)
       * @param time Amount of time of a full blink on and off cycle
       */
      void setColorState(ColorLED color, bool blink = false, float time = 0);
  };
};

#endif // LIGHT_BEACON