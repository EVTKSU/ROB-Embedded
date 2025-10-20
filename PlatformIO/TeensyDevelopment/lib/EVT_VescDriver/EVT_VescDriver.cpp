#include <EVT_SlewRateLimiter.hpp>
#include "EVT_VescDriver.h"
#include "EVT_RC.hpp"

VescUart vesc1;
VescUart vesc2;
String vescDebug = "";
String vesc1ErrorString;

const int neutral      = 990;
const int deadband     = 20;
const float maxRPM     = 7500.0f;
const int neutralBrake = 1030;
const int maxBrake     = 330;

const float forwardRange = (1700.0f - (neutral + deadband));
const float reverseRange = ((neutral - deadband) - 350.0f);
float revProp;

const float brakeRange = (float)(neutralBrake - maxBrake);
float brakeProp;

int ch_vesc;  // VESC RC transmitter channel value
int ch_brake; // Brake RC transmitter channel value

float maxChangeRPM = 500.0f; // Max RPM change per second
SlewRateLimiter rpmLimiter(maxChangeRPM);

bool inDeadband;

float rpmCommand; // Current RPM command
float rpmSetting; // Current RPM setting

float brakeCommand = 0.0; // Brake current setting in amps
bool brakingActive = (brakeCommand > 0.5f); // Small threshold to avoid noise

int mappedThrottle; // Mapped value of throttle percentage


void setupVesc() {
    Serial1.begin(115200);
    vesc1.setSerialPort(&Serial1);
    
    Serial5.begin(115200);
    vesc2.setSerialPort(&Serial5);
}


void updateVescControl() {
    // Read and clamp the raw SBUS channel value
    ch_vesc = constrain(channels[1], 350, 1700);
    ch_brake = constrain(channels[2], 330, 1700);

    // Throttle Mapping
    if (ch_vesc > neutral + deadband) { // Forward mapping
        rpmCommand = ((ch_vesc - (neutral + deadband)) / forwardRange) * maxRPM;
    } else if (ch_vesc < neutral - deadband) { // Reverse mapping
        revProp = ((neutral - deadband) - ch_vesc) / reverseRange;
        rpmCommand = -revProp * maxRPM;
    } else { // Within deadband, set RPM to zero
        rpmCommand = 0.0f;
    }


    // Brake Mapping
    if (ch_brake >= neutralBrake - maxBrake){
        brakeCommand = 0;
        Serial.println("No Brake!");
    } else {
        brakeProp  = (float)((neutralBrake - ch_brake) / brakeRange); // 0..1
        brakeCommand = brakeProp * 7.0f; // How many amps we want the brake current to be  

        vesc1.setBrakeCurrent(brakeCommand);
        vesc2.setBrakeCurrent(brakeCommand);

        Serial.print("Brake Current: ");
        Serial.print(brakeCommand);
    }


    // --- Slew‐rate limiting --- 
    rpmSetting = rpmLimiter.calculate(rpmCommand);

    // --- Coast in neutral, speed mode otherwise ---
    inDeadband = (ch_vesc >= neutral - deadband) && (ch_vesc <= neutral + deadband);


    // Prevent RPM commands this loop if braking is active
    brakingActive = (brakeCommand > 0.5f);
    if (brakingActive) {
        return; // Brake was already sent above. Return to avoid sending RPM in the same loop
    }


    if (inDeadband) {
        // Switch to current mode with 0A to avoid auto-braking in speed mode
        vesc1.setBrakeCurrent(0.0f);
        vesc2.setBrakeCurrent(0.0f);

        vesc1.setCurrent(0.0f); // If your wrapper lacks setCurrent(), use setDuty(0.0f) instead.
        vesc2.setCurrent(0.0f); // (or: vesc1.setDuty(0.0f); vesc2.setDuty(0.0f);)
    } else {
        vesc1.setRPM(rpmSetting);
        vesc2.setRPM(rpmSetting);

        Serial.print("Brake Current: ");
        Serial.print(brakeCommand);
        Serial.println();
    }
}


void updateVescControl(float throttlePercent) {
    // Map throttle_percent to [350, 1700]
    mappedThrottle = (int)(map(throttlePercent, -100, 100, 350, 1700));

    Serial.println();
    Serial.print("Mapped Throttle: ");
    Serial.print(mappedThrottle);

    Serial.print("Brake Current: ");
    Serial.print(brakeCommand);
    Serial.println();

    // Send the RPM command to the VESC
    vesc1.setRPM(mappedThrottle);
    vesc2.setRPM(mappedThrottle);
}


