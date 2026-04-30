#ifndef EVT_ETHERNET_H
#define EVT_ETHERNET_H

/*-----------------------------------------------------------------------------*/
/** 
 * @file   EVT_Ethernet.hpp
 * @brief  Header for EthernetEVT class
 * 
 * The EthernetEVT class is used to both send and receive UDP packets 
 * between the Teensy 4.1 and the Latte Panda Sigma.
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
      EthernetUDP udp; // One UDP socket used for both RX commands and TX telemetry

      IPAddress teensyIP {192, 168, 0, 177};              // Teensy 4.1 IP address 
      IPAddress pandaIP  {169, 254, 1, 10};              // Latte Panda Sigma IP address
      byte mac[6] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED}; // Teensy Ethernet MAC address 

      char autoBuffer[256];      // UDP command packet buffer from Latte Panda Sigma 
      char telemetryBuffer[256]; // Telemetry packet buffer from Teensy 4.1

      uint16_t autoPort = 8888;  // Autonomous command RX port
      uint16_t telemPort = 5005; // Telemetry TX port
    public:
      /**
       * @brief Sets up UDP Ethernet.
       */
      void setupUDP();

      /**
       * @brief Sends the telemetry packet.
       * 
       * @param error Boolean to denote an error in the low level 
       * @param state Current state of the State Machine
       * @param rpm VESC mechanical RPM
       * @param steering ODrive steering angle in degrees
       * @param oDrvVolt ODrive voltage
       * @param vescVolt VESC voltage
       * @param oDrvCurr ODrive current
       * @param vescCurr VESC current
       * @param oDrvTarget ODrive target steering angle in degrees
       * @param steer RC steering input
       * @param throttle RC throttle input
       * @param driveEncoderRevolutions Drive encoder revolutions accumulated since firmware startup
       * @param driveEncoderCount Raw drive encoder quadrature count accumulated since firmware startup
       */
      void sendTelemetry(
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
        double driveEncoderRevolutions,
        int32_t driveEncoderCount
      );

      /**
       * @brief Gets an autonomous command packet from the Panda as a string.
       * 
       * Expected autonomous command format:
       * rpm,steering_angle,brake_current,emergency_flag
       * 
       * @return The received command packet, or empty string if no packet exists.
       */
      std::string receiveUDP();
  };
}

#endif // EVT_ETHERNET_H
