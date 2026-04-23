#include <LightBeacon.hpp>

namespace Signals {
  LightBeacon::LightBeacon(ColorLED initColor, BeaconPins pins) {
    // Set the current color to the initial color 
    currentColor = initColor;
    colorBeacon = pins;

    // Initialize the beacon pins to output
    pinMode(colorBeacon.red, OUTPUT);
    pinMode(colorBeacon.green, OUTPUT);
    pinMode(colorBeacon.yellow, OUTPUT);

    // Set the inital color
    digitalWrite(colorBeacon.red, currentColor.red ? HIGH : LOW);
    digitalWrite(colorBeacon.green, currentColor.green ? HIGH : LOW);
    digitalWrite(colorBeacon.yellow, currentColor.yellow ? HIGH : LOW);
  }


  void LightBeacon::setColorState(ColorLED color, bool blink, float time) {
    if (blink && ((millis() - lastBlink) >= ((1'000 * time) / 2))) {
      isOn = !isOn;

      lastBlink = millis();
    } else if (!blink) {
      isOn = true;
    }

    currentColor.red = color.red;
    currentColor.green = color.green;
    currentColor.yellow = color.yellow;

    digitalWrite(colorBeacon.red, (currentColor.red && isOn) ? HIGH : LOW);
    digitalWrite(colorBeacon.green, (currentColor.green && isOn) ? HIGH : LOW);
    digitalWrite(colorBeacon.yellow, (currentColor.yellow && isOn) ? HIGH : LOW);
  }
}