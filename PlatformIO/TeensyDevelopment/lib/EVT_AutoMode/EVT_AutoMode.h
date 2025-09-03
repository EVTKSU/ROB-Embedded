#ifndef EVT_AUTOMODE_H
#define EVT_AUTOMODE_H

#include <Arduino.h>

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


#endif // EVT_AUTOMODE_H