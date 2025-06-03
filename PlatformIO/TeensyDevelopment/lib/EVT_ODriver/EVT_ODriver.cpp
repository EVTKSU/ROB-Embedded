#include "EVT_ODriver.h"
#include "EVT_RC.h"          // for channels[]

HardwareSerial &odrive_serial = Serial6;
ODriveUART    odrive(odrive_serial);

float target;                     // creating steering value to command ODrive
float absCenterPos    = 0.00f;     // ← NEW: manual “zero” reference
// ——— Constants ———
const float VEL_LIMIT    = 30.0f;    
const float ACCEL_LIMIT  = 20.0f;    
const float Two_pi       = 2.0f * 3.14159265358979323846f;
static const float MAX_STEERING_TURNS    = 5.50;  

// ——— State ———
bool          systemInitialized       = false;
String        odrvDebug;
float         SteeringCommandPosition = 0.0f;
float         lastTargetPosition      = 0.0f;
bool          errorClearFlag          = false;
unsigned long lastPrintTime           = 0;

// ——— Helpers ———
void configureAbsoluteReference(float absPos) {

    odrive_serial.println("w axis0.pos_vel_mapper.config.offset "       + String(absPos, 4));
    delay(20);
    odrive_serial.println("w axis0.pos_vel_mapper.config.offset_valid true");
    delay(20);
    odrive_serial.println("w axis0.pos_vel_mapper.config.approx_init_pos "       + String(absPos, 4));
    delay(20);
    odrive_serial.println("w axis0.pos_vel_mapper.config.approx_init_pos_valid true");
    delay(20);
    odrive_serial.println("w axis0.controller.config.absolute_setpoints true");
    delay(20);
}

void configureTrapTrajLimits() {
    Serial.println("Setting trap-traj vel/accel limits...");
    odrive_serial.println("w axis0.controller.config.vel_limit " + String(VEL_LIMIT));
    delay(20);
    odrive_serial.println("w axis0.controller.config.accel_limit " + String(ACCEL_LIMIT));
    delay(20);
}

//get active errors from ODrive, thanks odrive for making this so easy
int getActiveErrors() {
    // Path exactly as exposed in the firmware for active errors :contentReference[oaicite:0]{index=0}
    long errs = odrive.getParameterAsInt("axis0.active_errors");
    return (long)errs;
}

int getDisarmReason() {
    // Path exactly as exposed in the firmware for disarm reason :contentReference[oaicite:0]{index=0}
    long reason = odrive.getParameterAsInt("axis0.disarm_reason");
    return (long)reason;
}
int getInputMode() {
    // Path exactly as exposed in the firmware for input mode :contentReference[oaicite:0]{index=0}
    long input_mode = odrive.getParameterAsInt("axis0.controller.config.input_mode");
    return (long)input_mode;
}

// ——— Calibration & Homing ———
void initCalibration() {
    Serial.println("InitCalibration ▶ SBUS ch5");
    // Read current absolute position (in turns)
    

    // Motor & encoder offset calibration
    odrive.setState(AXIS_STATE_MOTOR_CALIBRATION);
    delay(4000);
    odrive.clearErrors();
    odrive.setState(AXIS_STATE_ENCODER_OFFSET_CALIBRATION);
    delay(4000);
    //this marks the end of the movement from calibration
    Serial.println("Motor & encoder offset calibration complete.");
    // Set axis state to IDLE
    odrive.setState(AXIS_STATE_IDLE);
    Serial.println("Axis set to IDLE. you have 8 seconds to manually home the motor.");

    delay(8000);
    
    ODriveFeedback fb = odrive.getFeedback();
    float absPos = fb.pos;
    Serial.print("encoder position: ");
    Serial.println(absPos, 4);
    delay(3000);

    absCenterPos = absPos; // Store the absolute position as the center reference

    Serial.print(" Center Position: ");
    Serial.println(absCenterPos, 4);

    // Engage closed-loop control
    unsigned long t0 = millis();
    while (odrive.getState() != AXIS_STATE_CLOSED_LOOP_CONTROL && millis() - t0 < 5000) {
        odrive.clearErrors();
        odrive.setState(AXIS_STATE_CLOSED_LOOP_CONTROL);
        delay(10);
    }
    Serial.println("Closed-loop CONTROL engaged.");

    

    // Switch to trapezoidal trajectory input mode
    odrive_serial.println("w axis0.controller.config.input_mode 1");
    delay(100);
    configureTrapTrajLimits(); // sets velocity and acceleration limits
    Serial.println("Trapezoidal trajectory input mode enabled.");
    // Store zero reference
    delay(1000);
    int current_mode = odrive.getParameterAsInt("axis0.controller.config.input_mode");
    Serial.print("Current input_mode = ");
    Serial.println(current_mode);
delay(1000);


    Serial.println("initCalibration ▶ complete");
}

// ——— Setup ———
void setupOdrv() {
    Serial.begin(115200);
    odrive_serial.begin(115200);
    Serial.println("ODrive serial init...");
    unsigned long t0 = millis();
    while (odrive.getState() == AXIS_STATE_UNDEFINED && millis() - t0 < 15000) {
        delay(50);
    }
    Serial.println(odrive.getState() == AXIS_STATE_UNDEFINED
                   ? "ODrive not found, proceeding standalone."
                   : "ODrive detected.");
    Serial.println("Awaiting SBUS ch5 to initCalibration.");
}

// ——— Control Loop ———
void updateOdrvControl() {
    // LED heartbeat until init
    static unsigned long ledT = 0;
    if (!systemInitialized && millis() - ledT > 500) {
        ledT = millis();
        digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN));
    } else if (systemInitialized) {
        digitalWrite(STATUS_LED_PIN, HIGH);
    }

    // Clear-errors on SBUS ch4
    int ch4 = channels[4];
    if (ch4 > 1500 && !errorClearFlag) {
        errorClearFlag     = true;
        Serial.println("SBUS4 ▶ clearErrors()");
        odrive.clearErrors();
        systemInitialized = false;
    }
    if (ch4 < 1500) {
        errorClearFlag = false;
    }

    // Trigger calibration/homing on ch5
    if (!systemInitialized) {
        if (channels[5] > 900) {
            initCalibration();
            systemInitialized = true;
        } else {
            Serial.print("Waiting ch5>900 ▶ ");
            Serial.println(channels[5]);
            return;
        }
    }

    // SBUS ch3 → offsetCmd (deadband + mapping), in radians
    int ch = constrain(channels[3], 377, 1763);
    const int neutral    = 1075;
    const int deadband   = 50;
    if (ch > (neutral + deadband)) {
        // Map [NEUTRAL+DEADBAND … SBUS_MAX] → [0 … +MAX_STEERING_TURNS]
        float fractionPos = float(ch - (neutral + deadband))
                          / float(1811 - (neutral + deadband));
        float steeringTurns = fractionPos * MAX_STEERING_TURNS;
        target = absCenterPos + steeringTurns;
    }
    else if (ch < (neutral - deadband)) {
        // Map [NEUTRAL‑DEADBAND … SBUS_MIN] → [0 … -MAX_STEERING_TURNS]
        float fractionNeg = float((neutral - deadband) - ch)
                          / float((neutral - deadband) - 350);
        float steeringTurns = fractionNeg * MAX_STEERING_TURNS;
        target = absCenterPos - steeringTurns;
    }
    else {
        // Within deadband: hold exactly at absCenterPos
        target = absCenterPos;
    }

    // Send position command (in turns) with velocity limit
    odrive.setPosition(target); // 0.0f for no torque feedforward

    long faults = getActiveErrors();
    long reason = getDisarmReason();
    long mode = getInputMode();
    // Print telemetry every 100 ms
    if (millis() - lastPrintTime > 100) {
        ODriveFeedback fb = odrive.getFeedback();
        Serial.print("Pos(turns): ");
        Serial.print(fb.pos, 6);
        Serial.print("Target()):");   Serial.print(target, 4);
        Serial.print("  Pos(turns):"); Serial.print(fb.pos, 4);
        Serial.print("  CH3:");        Serial.println(ch);
        Serial.print("Vel(rad/s):");  Serial.print(fb.vel, 4);
       // Serial.printf("Active errors: 0x%lX\n", faults);
       // Serial.printf("Disarm reason: 0x%lX\n", reason);
       // Serial.printf("Current input_mode = %lx\n", mode);
        lastPrintTime = millis();
    }
}

float getTarget() {
    return target;
}
