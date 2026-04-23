#ifndef EVT_TELEMETRY_H
#define EVT_TELEMETRY_H

/*-----------------------------------------------------------------------------*/
/** 
 * @file   EVT_Ethernet.hpp
 * @brief  Header for EthernetEVT class
 * 
 * The EthernetEVT class is used to both send and receive telemetry packets 
 * between the Teensy 4.1 and the Latte Panda Sigma 
 * 
 * @author Nyx Turbeville
 * @date   April 20, 2026
*//*---------------------------------------------------------------------------*/

#include <Arduino.h>
#include <NativeEthernet.h>
#include <NativeEthernetUdp.h>
#include <string>

namespace Signals {
  /**
   * @brief Class used for communications of UDP packets 
   */
  class EthernetEVT {
    private:
      EthernetUDP udp; // Ethernet class instance used to send and receive packets

      IPAddress teensyIP {192, 168, 0, 177};              // Teensy 4.1 IP address 
      IPAddress pandaIP  {192, 168, 0, 121};              // Latte Panda Sigma IP address
      byte mac[6] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED}; // Latte Panda Sigma MAC address 

      char autoBuffer[256];      // Telemetry packet buffer from Latte Panda Sigma 
      char telemetryBuffer[256]; // Telemetry packet buffer from Teensy 4.1

      uint16_t telemPort = 5005; // Telemetry UDP port 
    public:
      /**
       * @brief Sets up the UDP telemetry
       * 
       * @note Blocks the code from continuing until the ethernet is connected 
       */
      void setupUDP();


      /**
       * @brief Sends the telemetry packet
       * 
       * @param error Boolean to denote an error in the low level 
       * @param state Current state of the State Machine
       * @param rpm VESC rpm
       * @param steering ODrive steering 
       * @param oDrvVolt ODrive voltage
       * @param vescVolt VESC voltage
       * @param oDrvCurr ODrive current
       * @param vescCurr VESC current
       * @param oDrvTarget ODrive target position 
       * @param steer RC steering input
       * @param throttle RC throttle input
       */
      void sendTelemetry(bool error, const char * state, float rpm, float steering, float oDrvVolt, float vescVolt, float oDrvCurr, float vescCurr, float oDrvTarget, uint16_t steer, uint16_t throttle);


      /**
       * @brief Gets the telemetry packet from the Panda as a string 
       * 
       * @return The formatted telemetry packet 
       */
      std::string receiveUDP();
  };
}

#endif // EVT_TELEMETRY_H