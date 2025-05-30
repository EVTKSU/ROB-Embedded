#include "EVT_VescDriver.h"
#include "EVT_RC.h"

VescUart vesc1;
VescUart vesc2;
String vescDebug = "";
String vesc1ErrorString;




void setupVesc() {
    Serial1.begin(115200);
    vesc1.setSerialPort(&Serial1);
    
    Serial5.begin(115200);
    vesc2.setSerialPort(&Serial5);
}

void updateVescControl() {
    // Read and clamp the raw SBUS channel value
    int ch_vesc = constrain(channels[1], 350, 1700);
    
    const int neutral    = 990;
    const int deadband   = 20;
    const float maxRPM   = 7500.0f;
    
    float rpmCommand = 0.0f;

    // Forward mapping
    if (ch_vesc > neutral + deadband) {
        float forwardRange = (1700.0f - (neutral + deadband));
        rpmCommand = ((ch_vesc - (neutral + deadband)) / forwardRange) * maxRPM;
    }
    // Reverse mapping
    else if (ch_vesc < neutral - deadband) {
        float reverseRange = ((neutral - deadband) - 350.0f);
        float revProp = ((neutral - deadband) - ch_vesc) / reverseRange;
        rpmCommand = -revProp * maxRPM;
    }
    // Within deadband → zero
    else {
        rpmCommand = 0.0f;
    }

    // --- Approach A: Slew‐rate limiting ---
    static float lastRpm = 0.0f;
    const float maxDelta = 500.0f;  // max RPM change per loop
    float delta = rpmCommand - lastRpm;
    if (delta >  maxDelta) rpmCommand = lastRpm + maxDelta;
    if (delta < -maxDelta) rpmCommand = lastRpm - maxDelta;
    lastRpm = rpmCommand;

    /* 
    // --- Approach B: Exponential smoothing (alternative) ---
    // static float lastRpm = 0.0f;
    // const float alpha = 0.2f;  // between 0 (smooth) and 1 (responsive)
    // rpmCommand = alpha * rpmCommand + (1 - alpha) * lastRpm;
    // lastRpm    = rpmCommand;
    */

    // Send the RPM command to the VESC
    vesc1.setRPM(rpmCommand);
    vesc2.setRPM(rpmCommand);

    
    
}


