#include "EVT_AutoMode.hpp"


AutoMode::AutoMode(/* args */)
{
    float throttle = 0.0;   // Throttle value
    float steering = 0.0;   // Steering angle
    bool emergency = false; // Condition for emergency state
}

// Global variables for UDP control data
/*
char udpCopy[128];       // Modifiable UDP buffer
char *token;             // UDP string token
int currIndex;           // Current index
std::string rawCommands; // Raw UDP commands from the Panda
*/

void AutoMode::setControls(const std::string &udpData)
{
    strncpy(udpCopy, udpData.c_str(), sizeof(udpCopy) - 1);
    udpCopy[sizeof(udpCopy) - 1] = '\0'; // Ensure null termination

    token = strtok(udpCopy, ",");
    currIndex = 0;

    while (token != nullptr)
    {
        switch (currIndex)
        {
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

        currIndex++;
        token = strtok(nullptr, ",");
    }

    if (currIndex < 3)
    { // Prints a message to the console if the packet has less than 3 fields
        Serial.print("Malformed control packet (expected 3 fields): ");
        Serial.println(udpData.c_str());
    }
}

void AutoMode::updateAutonomousMode()
{
    // Set autonomous mode debug message.
    odrvDebug = "Autonomous mode active.";

    // Get the UDP commands from the Latte Panda
    rawCommands = receiveUdp();

    // Print the received values
    Serial.print(" | Throttle: ");
    Serial.print(throttle);
    Serial.print(" Steering: ");
    Serial.print(steering);
    Serial.print(" | Emergency: ");
    Serial.println(emergency ? "YES" : "NO");

    // Update the throttle value and send temeletry over UDP
    updateVescControl(throttle);
    sendTelemetry();

    // Only sets the controls if the commands aren't empty
    if (!rawCommands.empty())
    {
        setControls(rawCommands);
    }
}