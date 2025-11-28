
#include <SPI.h>
#include <sstream>
#include "EVT_VescDriver.h"
#include "EVT_ODriver.h"
#include "EVT_Ethernet.h"

// IN AUTO MODE, THERE IS NO REVERSE. REVERSE BRAKES IN THIS CASE.

// Global variables for UDP control data
float erpm = 0.0;
float throttle = 0.0;
float steering = 0.0;
float brakeCurrent = 0.0;
float throttleRpm = 0.0;
float SteeringPos = 0.0;
bool emergency = false;
bool brakeState = false;
bool coasting = false;
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


void CtrlVesc() {
if (emergency == true) {
        vesc1.setBrakeCurrent(20.0f); // set to max brake current
        return;
}
      if (throttle < 0.0f) {
        brakeCurrent = throttle *-1.0f / 5; // brake current = throttle value divided by 5. if max throttle is -100 then max brake current is 20A for now. val can be changed
        brakeState = true;
        vesc1.setBrakeCurrent(brakeCurrent);
        return;
    }

    else if (throttle == 0.0f) {
        coasting = true;  
        vesc1.setBrakeCurrent(0.0f);
        vesc1.setCurrent(0.0f);
        return;
    }

    else if (throttle > 0.0f) {
        throttleRpm = (throttle / 100.0f) * 7500.0f; // map throttle 0-100 to 0 - maxRPM (7500 for old vescrpm,14800 new theoretical vesc)
        brakeState = false;
        coasting = false;
        vesc1.setRPM(throttleRpm);
        return;
    }


}
void CtrlOdrive() {
    // Map steering (-100 to +100) to ODrive position range (-maxPos to +maxPos)
    if (steering <-0.25f){
        SteeringPos = (steering / 100.0f) * 2.25f; // map steering -100 to 0 to -maxPos to 0
    }
    else if (steering > 0.25f) {
        SteeringPos = (steering / 100.0f) * 2.25f; // map steering 0 to +100 to 0 to +maxPos
    }
    else {
        SteeringPos = 0.0f; // center position
    }
        
        odrive.trapezoidalMove(SteeringPos);
        
}


void updateAutonomousMode() {
    // Set autonomous mode debug message.
    odrvDebug = "Autonomous mode active.";
    
    std::string rawCommands = receiveUdp();

        Serial.print(" | Throttle(rpm): ");
        Serial.print(throttleRpm);
        Serial.print(" steering(turns): ");
        Serial.print(SteeringPos);
        Serial.print(" | Emergency: ");
        Serial.println(emergency ? "YES" : "NO");
        Serial.println(brakeState);
        Serial.println(coasting);


    CtrlVesc();
    CtrlOdrive();
    sendTelemetry();

    if (!rawCommands.empty()) {
        setControls(rawCommands);
    }

}