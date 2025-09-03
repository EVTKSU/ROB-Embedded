#include <SPI.h>
#include <cstdio>
#include <sstream>
#include <vector>
#include <cstdlib>

#include "EVT_Ethernet.h"
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


// Telemetry Values 
ODriveFeedback fb;     // ODrive Feedback class  
float steeringAngle;   // Current steering angle 
float target;          // Current ODrive target steering position 

const char *state;     // The current state as a string 

float odrvCurrent;     // Current ODrive current draw
float odrvVoltage;     // Current ODrive voltage 

float rpm;             // Current VESC RPM
float vescVoltage;     // Current VESC voltage 
float vescCurrent;     // Current VESC current draw

float rcSteeringInput; // Current steering input value
float rcThrottleInput; // Current throttle input value

int emergency;         // Emergency flag (Currently disabled)


// Telemetry destination details.
static IPAddress telemetryDestIP(192, 168, 0, 10);  // Panda IP
static const uint16_t TELEMETRY_DEST_PORT = 5005;   // Matches receiver

// UDP Packet Values 
int packetSize; // Size of the packet in bytes 
int len;        // Length of the packet buffer


void setupTelemetryUDP() {
  Serial.println("Initializing Telemetry UDP...");
  Ethernet.begin(mac, ip); // Begin Enternet communication 
  Udp.begin(8888);         // Begin UDP communication 
  
  // Checks for presence of Ethernet hardware
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("No Ethernet hardware found.");
  } else {
    Serial.println("Ethernet hardware is present.");
  }
  

  // Checks for Ethernet connection
  if (Ethernet.linkStatus() == LinkON) {
    Serial.println("Ethernet cable is connected (Link ON).");
  } else {
    Serial.println("Ethernet cable is not connected (Link OFF).");
  }

  delay(1000);
}


void sendTelemetry() {
  // Retrieve current ODrive feedback.
  fb = odrive.getFeedback();
  steeringAngle = fb.pos;
  target = getTarget(); // Get the current target position from ODrive

  // Get current system state
  state = StateToString(GetState());

  // Get ODrive telemetry
  odrvCurrent = odrive.getParameterAsFloat("ibus");
  odrvVoltage = odrive.getParameterAsFloat("vbus_voltage");
  
  // Get VESC telemetry
  rpm = vesc1.data.rpm;
  vescVoltage = vesc1.data.inpVoltage;
  vescCurrent = vesc1.data.avgInputCurrent + vesc2.data.avgInputCurrent;

  // Update RC data and sample channels
  updateSbusData();
  rcSteeringInput = channels[3];
  rcThrottleInput = channels[1];

  // Emergency flag (disabled for now)
  emergency = 0;

  // Format the telemetry string
  snprintf(
    telemetryPacketBuffer, 
    sizeof(telemetryPacketBuffer),
    "%d,%s,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f",
    emergency,
    state,
    rpm,
    steeringAngle,
    odrvVoltage,
    vescVoltage,
    odrvCurrent,
    vescCurrent,
    target,
    rcSteeringInput,
    rcThrottleInput
  );

  // Send via UDP
  Udp.beginPacket(telemetryDestIP, TELEMETRY_DEST_PORT);
  Udp.write(telemetryPacketBuffer);
  Udp.endPacket();
}


std::string receiveUdp() {
  packetSize = Udp.parsePacket();

  if (packetSize > 0) {
    len = Udp.read(autoBuffer, sizeof(autoBuffer) - 1);

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