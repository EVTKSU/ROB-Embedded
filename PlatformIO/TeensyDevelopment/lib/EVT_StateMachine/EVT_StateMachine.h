#ifndef EVT_STATEMACHINE_H
#define EVT_STATEMACHINE_H

#include <Arduino.h>

/**
 * @brief Enum representing the different states of the system.
 */
enum STATE {
    NONE,  // No state defined.
    INIT,  // Initialization state.
    IDLE,  // Idle state.  
    CALIB, // Calibration state.
    RC,    // Remote Control state.
    AUTO,  // Autonomous state.
    ERR,   // Error state.
};


const char *state_names[] = {
   "None",            // NONE
    "Initialization", // INIT
    "Idle",           // IDLE   <-- insert
    "Calibration",    // CALIB
    "RC",             // RC
    "Autonomous",     // AUTO
    "Error"           // ERR
    // (no entry for STATE_COUNT)
};


extern STATE CurrentState; // Current state of the system (Accessible by other modules)


/**
 * @brief Retrieves the current state.
 * 
 * @return The current system state.
 */
STATE GetState();


/**
 * @brief Updates the global state and prints a message via Serial.
 * 
 * @note If the new state is ERR, it prints an error message.
 * 
 * @param newState The new state to set
 */
void SetState(STATE newState);


/**
 * @brief Sets the error state and prints detailed error information.
 * 
 * @param location A C-style string indicating where the error occurred.
 * @param reason A C-style string explaining the reason for the error.
 */
void SetErrorState(const char* location, const char* reason);


/**
 * @brief Converts a STATE enum value to a C-style string.
 *
 * @param s The state enum value.
 * @return A C-style string representing the state.
 */
const char* StateToString(STATE s);


/**
 * @brief Prints the current state to the Serial Monitor
 */
void PrintState();


// Error location constants
static const char* const ERR_VESC     = "vesc";
static const char* const ERR_ODRIVE   = "odrive";
static const char* const ERR_SBUS     = "sbus";
static const char* const ERR_ETHERNET = "ethernet";

// VESC fault reason constants
static const char* const ERR_VESC_FAULT_CODE_NONE             = "FAULT_CODE_NONE";             // 0
static const char* const ERR_VESC_FAULT_CODE_UNDER_VOLTAGE    = "FAULT_CODE_UNDER_VOLTAGE";    // 2
static const char* const ERR_VESC_FAULT_CODE_DRV              = "FAULT_CODE_DRV";              // 3
static const char* const ERR_VESC_FAULT_CODE_ABS_OVER_CURRENT = "FAULT_CODE_ABS_OVER_CURRENT"; // 4
static const char* const ERR_VESC_FAULT_CODE_OVER_TEMP_FET    = "FAULT_CODE_OVER_TEMP_FET";    // 5

#endif // EVT_STATEMACHINE_H