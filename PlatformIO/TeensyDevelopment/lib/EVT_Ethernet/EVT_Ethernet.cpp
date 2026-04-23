#include <EVT_Ethernet.hpp>
#include <EVT_RC.hpp>

#include <SPI.h>
#include <cstdio>
#include <sstream>
#include <vector>
#include <cstdlib>

#include "ModuleConstants.hpp"
using namespace Constants;

namespace Signals {
  void EthernetEVT::setupUDP() {
    if (Serial) {
      Serial.println("Initializing UDP telemetry");
    }

    // Begin Ethernet communication to the Latte Panda Sigma  
    Ethernet.begin(mac, teensyIP);
    udp.begin(8888);

    if (Serial) {
      if (Ethernet.hardwareStatus() == EthernetNoHardware) {
        Serial.println("No Ethernet hardware found");
      } else {
        Serial.println("Ethernet hardware is present");
      }

      if (Ethernet.linkStatus() == LinkON) {
        Serial.println("Ethernet cable is connected");
      } else {
        Serial.println("Ethernet cable is not connected");
      }
    } 
  }


  void EthernetEVT::sendTelemetry(bool error, const char * state, float rpm, float steering, float oDrvVolt, float vescVolt, float oDrvCurr, float vescCurr, float oDrvTarget, uint16_t steer, uint16_t throttle) {
    // Write the necessary values to the packet 
    snprintf(
      telemetryBuffer,                                         // Packet to be written
      sizeof(telemetryBuffer),                                 // Size of the allowed packet 
      "%d,%s,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%d,%d", // Formatted packet string
      error,                                                   // Emergency flag
      state,                                                   // Current State
      rpm,                                                     // VESC RPM target
      steering,                                                // ODrive position
      oDrvVolt,                                                // ODrive voltage
      vescVolt,                                                // VESC voltage
      oDrvCurr,                                                // ODrive current
      vescCurr,                                                // VESC current 
      oDrvTarget,                                              // Target steering position
      steer,                                                   // RC steering input position
      throttle                                                 // RC throttle input ERPM
    );

    // Prints the telemetry packet to the Serial Monitor 
    if (Serial && IOConstants::telemetryToSerial) {
      Serial.println(telemetryBuffer);
    }

    // Write the telemetry packet from the Teensy 4.1 to the Latte Panda Sigma 
    udp.beginPacket(pandaIP, telemPort);
    udp.write(telemetryBuffer);
    udp.endPacket();
  }


  std::string EthernetEVT::receiveUDP() {
    if (udp.parsePacket() > 0) {
      // Add a stop bit to the packet 
      if (udp.read(autoBuffer, sizeof(autoBuffer - 1)) > 0) {
        autoBuffer[udp.read(autoBuffer, sizeof(autoBuffer - 1))] = '\0';
      }

      // Print the received packet to the Serial Monitor
      if (Serial && IOConstants::telemetryToSerial) {
        Serial.print("Received Packet: ");
        Serial.println(autoBuffer);
      }

      // Return the received packet 
      return std::string(autoBuffer);
    }

    // Return an empty string if no packet is received 
    return std::string();
  }
}
