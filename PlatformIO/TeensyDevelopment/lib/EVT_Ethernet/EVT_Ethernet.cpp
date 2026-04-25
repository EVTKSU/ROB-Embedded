#include <EVT_Ethernet.hpp>
#include <EVT_RC.hpp>

#include <SPI.h>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <vector>
#include <cstdlib>

#include "ModuleConstants.hpp"
using namespace Constants;

namespace Signals {
  void EthernetEVT::setupUDP() {
    if (Serial) {
      Serial.println("Initializing UDP Ethernet");
    }

    // Begin Ethernet communication to the Latte Panda Sigma.
    Ethernet.begin(mac, teensyIP);

    // One UDP object is used for both:
    //   RX autonomous command packets on autoPort
    //   TX telemetry packets to pandaIP:telemPort
    udp.begin(autoPort);

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

      Serial.print("Teensy IP: ");
      Serial.println(Ethernet.localIP());

      Serial.print("Listening for autonomous UDP commands on port ");
      Serial.println(autoPort);

      Serial.print("Sending telemetry to ");
      Serial.print(pandaIP);
      Serial.print(":");
      Serial.println(telemPort);
    }
  }


  void EthernetEVT::sendTelemetry(
    bool error,
    const char * state,
    float rpm,
    float steering,
    float oDrvVolt,
    float vescVolt,
    float oDrvCurr,
    float vescCurr,
    float oDrvTarget,
    uint16_t steer,
    uint16_t throttle,
    double driveEncoderPosition
  ) {
    snprintf(
      telemetryBuffer,
      sizeof(telemetryBuffer),
      "%d,%s,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%d,%d,%0.4f",
      error,
      state,
      rpm,
      steering,
      oDrvVolt,
      vescVolt,
      oDrvCurr,
      vescCurr,
      oDrvTarget,
      steer,
      throttle,
      driveEncoderPosition
    );

    // Print the telemetry packet to the Serial Monitor.
    if (Serial && IOConstants::telemetryToSerial) {
      Serial.println(telemetryBuffer);
    }

    // Write the telemetry packet from the Teensy 4.1 to the Latte Panda Sigma.
    udp.beginPacket(pandaIP, telemPort);
    udp.write(
      reinterpret_cast<const uint8_t *>(telemetryBuffer),
      strlen(telemetryBuffer)
    );
    udp.endPacket();
  }


  std::string EthernetEVT::receiveUDP() {
    int packetSize = udp.parsePacket();

    if (packetSize <= 0) {
      return std::string();
    }

    // Read the UDP datagram once into the full buffer.
    // Leave one byte open for the null terminator.
    int len = udp.read(autoBuffer, sizeof(autoBuffer) - 1);

    if (len <= 0) {
      autoBuffer[0] = '\0';

      if (Serial && IOConstants::telemetryToSerial) {
        Serial.println("Received empty UDP packet");
      }

      return std::string();
    }

    autoBuffer[len] = '\0';

    // If a packet is somehow longer than autoBuffer, drain the leftover bytes.
    while (udp.available() > 0) {
      udp.read();
    }

    // Strip newline / carriage return if a sender includes them.
    for (int i = 0; i < len; i++) {
      if (autoBuffer[i] == '\r' || autoBuffer[i] == '\n') {
        autoBuffer[i] = '\0';
        break;
      }
    }

    // Print the received packet to the Serial Monitor.
    if (Serial && IOConstants::telemetryToSerial) {
      Serial.print("Received Packet: ");
      Serial.println(autoBuffer);
    }

    return std::string(autoBuffer);
  }
}