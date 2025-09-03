#ifndef EVT_VESCDRIVER_H
#define EVT_VESCDRIVER_H

#include <Arduino.h>
#include <VescUart.h>
#include <SoftwareSerial.h>

extern VescUart vesc1; // Control over UART for left motor
extern VescUart vesc2; // Control over UART for right motor

extern String vescDebug;        // Debug string
extern String vesc1ErrorString; // Error string for left VESC 


/**
 * @brief Converts a received VESC error code to a string for debugging
 * 
 * @param code mc_fault_code received from the VESC
 * @return Fault code as a readable string 
 */
inline String vescErrorToString(uint8_t code) {
    switch (code) {
        case (mc_fault_code::FAULT_CODE_NONE):  return "FAULT_CODE_NONE";
        case (mc_fault_code::FAULT_CODE_OVER_VOLTAGE):  return "FAULT_CODE_OVER_VOLTAGE";
        case (mc_fault_code::FAULT_CODE_UNDER_VOLTAGE):  return "FAULT_CODE_UNDER_VOLTAGE";
        case (mc_fault_code::FAULT_CODE_DRV):  return "FAULT_CODE_DRV";
        case (mc_fault_code::FAULT_CODE_ABS_OVER_CURRENT):  return "FAULT_CODE_ABS_OVER_CURRENT";
        case (mc_fault_code::FAULT_CODE_OVER_TEMP_FET):  return "FAULT_CODE_OVER_TEMP_FET";
        case (mc_fault_code::FAULT_CODE_OVER_TEMP_MOTOR):  return "FAULT_CODE_OVER_TEMP_MOTOR";
        case (mc_fault_code::FAULT_CODE_GATE_DRIVER_OVER_VOLTAGE):  return "FAULT_CODE_GATE_DRIVER_OVER_VOLTAGE";
        case (mc_fault_code::FAULT_CODE_GATE_DRIVER_UNDER_VOLTAGE):  return "FAULT_CODE_GATE_DRIVER_UNDER_VOLTAGE";
        case (mc_fault_code::FAULT_CODE_MCU_UNDER_VOLTAGE):  return "FAULT_CODE_MCU_UNDER_VOLTAGE";
        case (mc_fault_code::FAULT_CODE_BOOTING_FROM_WATCHDOG_RESET): return "FAULT_CODE_BOOTING_FROM_WATCHDOG_RESET";
        case (mc_fault_code::FAULT_CODE_ENCODER_SPI): return "FAULT_CODE_ENCODER_SPI";
        case (mc_fault_code::FAULT_CODE_ENCODER_SINCOS_BELOW_MIN_AMPLITUDE): return "FAULT_CODE_ENCODER_SINCOS_BELOW_MIN_AMPLITUDE";
        default: return "UNKNOWN_FAULT_" + String(code);
    }
}

//----------------------------------------------------------------------
// Call this once per loop to refresh the two global error strings
//----------------------------------------------------------------------
//----------------------------------------------------------------------
// Your existing VESC setup/control APIs
//----------------------------------------------------------------------


/**
 * @brief Begin UART communication to the VESCs 
 */
void setupVesc();


/**
 * @brief Update the throttle or braking of the VESCs using the SBUS values 
 */
void updateVescControl();


/**
 * @brief Update the throttle of the VESCs using the given percentage
 * 
 * @note This method does not impelment braking capabilites 
 * 
 * @param throttlePercent Throttle sent to the VESC as a percentage of max RPM
 */
void updateVescControl(float throttlePercent);


#endif // EVT_VESCDRIVER_H
