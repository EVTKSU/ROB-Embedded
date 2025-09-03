#ifndef EVT_TELEMETRY_H
#define EVT_TELEMETRY_H

#include <Arduino.h>
#include <NativeEthernet.h>
#include <NativeEthernetUdp.h>
#include <string>

// Global Telemetry objects and variables.
extern EthernetUDP Udp;
extern IPAddress ip;
extern byte mac[];


/**
 * @brief Setup method for initialization of Ethernet and UDP communications 
 */
void setupTelemetryUDP();


/**
 * @brief Send telemetry data over UDP and display the values on the Serial Monitor 
 */
void sendTelemetry();


/**
 * @brief Receives the UDP value from the Latte Panda 
 * 
 * @return The received UDP string 
 */
std::string receiveUdp();


#endif // EVT_TELEMETRY_H