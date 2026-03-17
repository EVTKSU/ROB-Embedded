#include <EVT_RC.hpp>
#include "TransmitterConstants.hpp"

namespace Signals {
  ControlRC::ControlRC() {
    Serial1.begin(100'000, SERIAL_8E2); // Begin the sBus serial port
    sBus.begin();                       // Begin the sBus communication 

    delay(500);
  }


  bool ControlRC::update() {
    return sBus.read(channelVal, &sBusFailsafe, &sBusLostFrame);
  }


  void ControlRC::setMapping(const uint16_t mapArray[], mapType mappingType) {
    switch (mappingType) {
      case (mapType::JOYSTICK):
        joystickMap[0] = mapArray[0];
        joystickMap[1] = mapArray[1];
        
        break;
      case (mapType::SWITCH):
        switchMap[0] = mapArray[0];
        switchMap[1] = mapArray[1];

        break;
      case (mapType::TRI_SWITCH):
        triSwitchMap[0] = mapArray[0];
        triSwitchMap[1] = mapArray[1];
        triSwitchMap[2] = mapArray[2];

        break;
      case (mapType::KNOB):
        knobMap[0] = mapArray[0];
        knobMap[1] = mapArray[1];

        break;
    }
  }


  uint16_t ControlRC::getChannelValue(ChannelRC channel, bool mapChannel) {
    if (mapChannel) {
      switch (channel) {
        case (ChannelRC::LEFT_X):
        case (ChannelRC::RIGHT_X):
        case (ChannelRC::RIGHT_Y):
        case (ChannelRC::LEFT_Y):
          return map(channelVal[channel], TransmitterConstants::minRC, TransmitterConstants::maxRC, joystickMap[0], joystickMap[1]);
        case (ChannelRC::SWA):
        case (ChannelRC::SWB):
        case (ChannelRC::SWD):
        case (ChannelRC::SWF):
        case (ChannelRC::SWH):
          return map(channelVal[channel], TransmitterConstants::minRC, TransmitterConstants::maxRC, switchMap[0], switchMap[1]);
        case (ChannelRC::SWC):
        case (ChannelRC::SWE):
        case (ChannelRC::SWG):
          if (channelVal[channel] == TransmitterConstants::minRC) {
            return triSwitchMap[0];
          } else if (channelVal[channel] == (TransmitterConstants::midRC)) {
            return triSwitchMap[1];
          } else {
            return triSwitchMap[2];
          }
        case (ChannelRC::VRA):
        case (ChannelRC::VRB):
        case (ChannelRC::VRC):
        case (ChannelRC::VRD):
          return map(channelVal[channel], TransmitterConstants::minRC, TransmitterConstants::maxRC, knobMap[0], knobMap[1]);
        default:
          return channelVal[channel];
      }
    } 
    
    return channelVal[channel];
  }


  uint16_t * ControlRC::getValueArray() {
    return channelVal;
  }


  void ControlRC::printChannels(bool isMapped) {
    for (int i = 0; i < TransmitterConstants::numChannels; i++) {
      Serial.print((i + 1) < 10 ? "Ch[0" : "Ch[");
      Serial.print(i + 1);
      Serial.print("] - ");
      Serial.print(getChannelValue((ChannelRC)i, isMapped));
      Serial.print(i < TransmitterConstants::numChannels - 1 ? "\t| " : "\n");
    }
  }


  void ControlRC::printChannel(ChannelRC channel, bool isMapped) {
    Serial.print(int(channel + 1) < 10 ? "Ch[0" : "Ch[");
    Serial.print(int(channel + 1));
    Serial.print("] - ");
    Serial.println(getChannelValue(channel, isMapped));
  }


  bool ControlRC::mapSwitches(uint16_t val) {
    return val == TransmitterConstants::minRC ? false : true;
  }


  template <class T>
  T ControlRC::mapToPercent(T value, const T (&mapArray)[2]) {
    return map(
      value,
      mapArray[0],
      mapArray[1],
      0,
      100
    );
  }
}