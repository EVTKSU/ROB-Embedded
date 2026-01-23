#ifndef EVT_RC_H
#define EVT_RC_H

#include <Arduino.h>
#include <SBUS.h>

// Raw SBUS channels (0-based indices: channels[0]..channels[15])
extern uint16_t channels[16];

// Named switch aliases (these are updated by updateSbusData())
extern uint16_t auto_switch;
extern uint16_t calibration_switch;
extern uint16_t reset_switch;

// Optional status flags
extern bool sbusFailSafe;
extern bool sbusLostFrame;

// Public API
void setupSbus();
bool updateSbusData();

#endif // EVT_RC_H
