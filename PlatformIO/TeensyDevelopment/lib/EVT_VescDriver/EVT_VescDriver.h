#ifndef EVT_VESCDRIVER_H
#define EVT_VESCDRIVER_H

#include <Arduino.h>
#include <VescUart.h>
#include <SoftwareSerial.h>

//----------------------------------------------------------------------
// VESC UART instances (defined in your .cpp)
//----------------------------------------------------------------------
extern VescUart vesc1;
extern VescUart vesc2;

//----------------------------------------------------------------------
// Debug & Error Strings (defined in your .cpp)
//----------------------------------------------------------------------
extern String vescDebug;
extern String vesc1ErrorString;

//----------------------------------------------------------------------
// Map a raw mc_fault_code into a human‑readable string
//----------------------------------------------------------------------
inline String vescErrorToString(uint8_t code) {
    switch (code) {
        case 0:  return "FAULT_CODE_NONE";
        case 1:  return "FAULT_CODE_OVER_VOLTAGE";
        case 2:  return "FAULT_CODE_UNDER_VOLTAGE";
        case 3:  return "FAULT_CODE_DRV";
        case 4:  return "FAULT_CODE_ABS_OVER_CURRENT";
        case 5:  return "FAULT_CODE_OVER_TEMP_FET";
        case 6:  return "FAULT_CODE_OVER_TEMP_MOTOR";
        case 7:  return "FAULT_CODE_GATE_DRIVER_OVER_VOLTAGE";
        case 8:  return "FAULT_CODE_GATE_DRIVER_UNDER_VOLTAGE";
        case 9:  return "FAULT_CODE_MCU_UNDER_VOLTAGE";
        case 10: return "FAULT_CODE_BOOTING_FROM_WATCHDOG_RESET";
        case 11: return "FAULT_CODE_ENCODER_SPI";
        case 12: return "FAULT_CODE_ENCODER_SINCOS_BELOW_MIN_AMPLITUDE";
        default: return "UNKNOWN_FAULT_" + String(code);
    }
}

//----------------------------------------------------------------------
// Call this once per loop to refresh the two global error strings
//----------------------------------------------------------------------
//----------------------------------------------------------------------
// Your existing VESC setup/control APIs
//----------------------------------------------------------------------
void setupVesc();
void updateVescControl();
void updateVescControl(float throttle_percent);
void getEncoder(float &wheelPos, float &mph);

#endif // EVT_VESCDRIVER_H
