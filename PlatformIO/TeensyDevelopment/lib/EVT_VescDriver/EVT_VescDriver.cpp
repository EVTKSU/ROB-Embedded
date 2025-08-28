#include "EVT_VescDriver.h"
#include "EVT_RC.h"

VescUart vesc1;
VescUart vesc2;
String vescDebug = "";
String vesc1ErrorString;


float brakeCommand = 0.0;


void setupVesc() {
    Serial1.begin(115200);
    vesc1.setSerialPort(&Serial1);
    
    Serial5.begin(115200);
    vesc2.setSerialPort(&Serial5);
}

void updateVescControl() {
    // Read and clamp the raw SBUS channel value
    int ch_vesc = constrain(channels[1], 350, 1700);
    int ch_brake = constrain(channels[2], 330, 1700);

    
    const int neutral    = 990;
    const int deadband   = 20;
    const float maxRPM   = 7500.0f;
    const int neutral_brake = 1030;
    const int brake_max = 330;



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



    // // brake map

    if (ch_brake >= neutral_brake - brake_max){
    
       brakeCommand = 0;
        Serial.println("no brake!");
    
    }
    
    else{

    float brakeRange = float(neutral_brake - brake_max);     // e.g., 1030 - 330 = 700
    float brakeProp  = float(neutral_brake - ch_brake) / brakeRange; // 0..1
    brakeCommand = brakeProp * 40.0f;                        
    vesc1.setBrakeCurrent(brakeCommand);
    vesc2.setBrakeCurrent(brakeCommand);
       Serial.print("brake current");
    Serial.print(brakeCommand);
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

 // --- NEW: Coast in neutral, speed mode otherwise ---
bool inDeadband = (ch_vesc >= neutral - deadband) && (ch_vesc <= neutral + deadband);

if (inDeadband) {
    // Switch to current mode with 0 A to avoid auto-braking in speed mode
    vesc1.setBrakeCurrent(0.0f);
    vesc2.setBrakeCurrent(0.0f);
    vesc1.setCurrent(0.0f);     // If your wrapper lacks setCurrent(), use setDuty(0.0f) instead.
    vesc2.setCurrent(0.0f);     // (or: vesc1.setDuty(0.0f); vesc2.setDuty(0.0f);)
} else {
    
    vesc1.setRPM(rpmCommand);
    vesc2.setRPM(rpmCommand);
    Serial.print("brake current");
    Serial.print(brakeCommand);
    Serial.println();

}
}

void updateVescControl(float throttle_percent) {

    // mappint -100 -> +100 to 350 -> 1700
    int mapped_throttle = int(throttle_percent * 75);
    Serial.println();
    Serial.print("mapped throttle: ");
    Serial.print(mapped_throttle);
    Serial.print("brake current");
    Serial.print(brakeCommand);
    Serial.println();

    // Send the RPM command to the VESC
    vesc1.setRPM(mapped_throttle);
    vesc2.setRPM(mapped_throttle);

    
    
}


