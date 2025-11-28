
#include <SPI.h>
#include <sstream>
#include "EVT_VescDriver.h"
#include "EVT_ODriver.h"
#include "EVT_Ethernet.h"

// Global variables for UDP control data
float throttle = 0.0;
float steering = 0.0;
bool emergency = false;

// Function to parse UDP data and update control variables
// Expected format: "throttle,steering,emergency"
void setControls(const std::string &udpData) {
    // Copy string to modifiable buffer
    char udpCopy[128];
    strncpy(udpCopy, udpData.c_str(), sizeof(udpCopy) - 1);
    udpCopy[sizeof(udpCopy) - 1] = '\0';  // Ensure null termination

    char* token = strtok(udpCopy, ",");
    int index = 0;

    while (token != nullptr) {
        switch (index) {
            case 0:
                throttle = atof(token);
                break;
            case 1:
                steering = atof(token);
                break;
            case 2:
                emergency = (atoi(token) != 0);
                break;
        }

        index++;
        token = strtok(nullptr, ",");
    }

    if (index < 3) {
        Serial.print("Malformed control packet (expected 3 fields): ");
        Serial.println(udpData.c_str());
    }

}


void updateAutonomousMode() {
    // Set autonomous mode debug message.
    odrvDebug = "Autonomous mode active.";
    
    std::string rawCommands = receiveUdp();

        Serial.print(" | Throttle: ");
        Serial.print(throttle);
        Serial.print(" Steering: ");
        Serial.print(steering);
        Serial.print(" | Emergency: ");
        Serial.println(emergency ? "YES" : "NO");

    updateVescControl(throttle);
    sendTelemetry();

    if (!rawCommands.empty()) {
        setControls(rawCommands);
    }

}