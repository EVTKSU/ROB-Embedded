#ifndef EVT_TELEMETRY_H
#define EVT_TELEMETRY_H

#include <Arduino.h>
#include <NativeEthernet.h>
#include <NativeEthernetUdp.h>
#include <string>

/*
// Global Telemetry objects and variables.
extern EthernetUDP Udp;
extern IPAddress ip;
extern byte mac[];

// Telemetry function prototypes.
void setupTelemetryUDP();
void sendTelemetry();
std::string receiveUdp();
*/

namespace Signals {
  class EthernetEVT {
    private:
      EthernetUDP udp;

      IPAddress teensyIP {192, 168, 0, 177};
      IPAddress pandaIP  {192, 168, 0, 121};

      byte mac[6] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

      char autoBuffer[256];
      char telemetryBuffer[256];

      uint16_t telemPort = 5005;
    public:
      /**
       * @brief Sets up the UDP telemetry
       * 
       * @note Blocks the code from continuing until the ethernet is connected 
       */
      void setupUDP();


      /**
       * @brief Sends the telemetry packet 
       */
      void sendTelemetry();


      /**
       * @brief Gets the telemetry packet from the Panda as a string 
       * 
       * @return The formatted telemetry packet 
       */
      std::string receiveUDP();
  };
}

#endif // EVT_TELEMETRY_H
