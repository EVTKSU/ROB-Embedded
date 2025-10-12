#ifndef EVT_AUTOMODE_H
#define EVT_AUTOMODE_H

#include <Arduino.h>
#include <SPI.h>
#include <sstream>

#include "EVT_VescDriver.h"
#include "EVT_ODriver.h"
#include "EVT_Ethernet.h"

class AutoMode
{
private:
// Global variables for UDP control data
float throttle;     // Throttle value 
float steering;     // Steering angle
bool emergency;   // Condition for emergency state

char udpCopy[128];       // Modifiable UDP buffer
char* token;             // UDP string token 
int currIndex;           // Current index 
std::string rawCommands; // Raw UDP commands from the Panda

public:
AutoMode();
/**
 * @brief Function to parse recieved UDP data and update controls accordingly
 *
 * @note Expected UDP format -- "{throttle},{steering},{emergency}"
 *
 * @param udpData Data recieved from Ethernet
 */
void setControls(const std::string &udpData);

/**
 * @brief Update the controls with the current UDP and telemetry data
 */
void updateAutonomousMode();

};





#endif // EVT_AUTOMODE_H