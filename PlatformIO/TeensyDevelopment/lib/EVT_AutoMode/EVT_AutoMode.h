#ifndef EVT_AUTOMODE_H
#define EVT_AUTOMODE_H

#include <Arduino.h>
#include <string>

// Autonomous mode function prototype.
void updateAutonomousMode();
void setControls(const std::string &udpData);
void runMappedControls();
void CtrlVesc();
void CtrlOdrive();
#endif // EVT_AUTOMODE_H
