#include "EVT_Ethernet.h"
#include <SPI.h>
#include <cstdio>
#include <sstream>
#include <vector>
#include <cstdlib>
#include "EVT_RC.h"
#include "EVT_StateMachine.h"
#include "EVT_VescDriver.h"
#include "EVT_ODriver.h"

// Global object definitions.
EthernetUDP Udp;
IPAddress ip(192, 168, 0, 177);
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// Internal buffers for UDP packets.
char autoBuffer[256];
static char telemetryPacketBuffer[256];


// Telemetry destination details.
static IPAddress telemetryDestIP(192, 168, 0, 10);  // Panda IP
static const uint16_t TELEMETRY_DEST_PORT = 5005;  // Matches receiver

// Setup function for initializing Ethernet and UDP.
void setupTelemetryUDP() {
  Serial.println("Initializing Telemetry UDP...");
  Ethernet.begin(mac, ip);
  Udp.begin(8888);
  
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("No Ethernet hardware found.");
  } else {
    Serial.println("Ethernet hardware is present.");
  }
  
  if (Ethernet.linkStatus() == LinkON) {
    Serial.println("Ethernet cable is connected (Link ON).");
  } else {
    Serial.println("Ethernet cable is not connected (Link OFF).");
  }
  delay(1000);
}


// Function to send telemetry data over UDP and display on Serial.
// Function to send telemetry data over UDP and display on Serial.
void sendTelemetry() {
    // Retrieve current ODrive feedback.
    ODriveFeedback fb = odrive.getFeedback();
    float steeringAngle = fb.pos;
    float target = getTarget(); // Get the current target position from ODrive
 
    // Get current system state
    const char* state = StateToString(GetState());

    // Get ODrive telemetry
    float odrvCurrent = odrive.getParameterAsFloat("ibus");
    float odrvVoltage = odrive.getParameterAsFloat("vbus_voltage");
    
    // Get VESC telemetry
    float rpm = vesc1.data.rpm;
    float vescVoltage = vesc1.data.inpVoltage;
    float vescCurrent = vesc1.data.avgInputCurrent;

    // Update RC data and sample channels
    updateSbusData();
    float rcSteeringInput = channels[3];
    float rcThrottleInput = channels[1];

    // Emergency flag (disabled for now)
    int emergency = 0;

    // Format the telemetry string
    snprintf(telemetryPacketBuffer, sizeof(telemetryPacketBuffer),
             "%d,%s,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f",
             emergency,
             state,
             rpm,
             steeringAngle,
             odrvVoltage,
             vescVoltage,
             odrvCurrent,
             vescCurrent,
             target,  // target is the steering position from odrive
             rcSteeringInput,
             rcThrottleInput);

    // Send via UDP
    Udp.beginPacket(telemetryDestIP, TELEMETRY_DEST_PORT);
    Udp.write(telemetryPacketBuffer);
    Udp.endPacket();
}


std::string receiveUdp() {
  int packetSize = Udp.parsePacket();
  if (packetSize > 0) {
    int len = Udp.read(autoBuffer, sizeof(autoBuffer) - 1);
    if (len > 0) {
      autoBuffer[len] = '\0';
    }
    Serial.print("Received packet: ");
    Serial.println(autoBuffer);
    return std::string(autoBuffer);
  }
  // Return an empty string if no packet is received.
  return std::string();
}


