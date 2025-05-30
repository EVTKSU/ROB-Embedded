#include "EVT_ODriver.h"
#include "EVT_RC.h"          // for channels[]

HardwareSerial &odrive_serial = Serial6;
ODriveUART    odrive(odrive_serial);

float target;  // creating steering value to command ODrive

// ——— Constants ———
const float MEMORY_ZERO  = 1.77f;    // hard‑coded zero position (in turns)
const float VEL_LIMIT    = 5.0f;    // rad/s
const float ACCEL_LIMIT  = 1.0f;    // rad/s²
const float Two_pi       = 2.0f * 3.14159265358979323846f;

// ——— State ———
bool          systemInitialized       = false;
String        odrvDebug;
float         SteeringCommandPosition = MEMORY_ZERO;
float         lastTargetPosition      = MEMORY_ZERO;
bool          errorClearFlag          = false;
unsigned long lastPrintTime           = 0;

// ——— Helpers ———
void configureAbsoluteReference(float absPos) {
    Serial.print("Configuring ABS ref at: ");
    Serial.println(absPos, 4);
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
    odrive_serial.println("w axis0.trap_traj.config.vel_limit "   + String(VEL_LIMIT));
    delay(20);
    odrive_serial.println("w axis0.trap_traj.config.accel_limit " + String(ACCEL_LIMIT));
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

// ——— Calibration & Homing ———
void initCalibration() {
    Serial.println("InitCalibration ▶ SBUS ch5");
    // Read current absolute position (in turns)
    ODriveFeedback fb = odrive.getFeedback();
    float absPos = fb.pos;
    Serial.print("pre‑cal abs pos: ");
    Serial.println(absPos, 4);
    delay(3000);

    // Motor & encoder offset calibration
    odrive.setState(AXIS_STATE_MOTOR_CALIBRATION);
    delay(4000);
    odrive.clearErrors();
    odrive.setState(AXIS_STATE_ENCODER_OFFSET_CALIBRATION);
    delay(4000);

    // Configure absolute reference frame
    configureAbsoluteReference(absPos);

    // Set velocity/accel caps
    configureTrapTrajLimits();

    // Engage closed-loop control
    unsigned long t0 = millis();
    while (odrive.getState() != AXIS_STATE_CLOSED_LOOP_CONTROL && millis() - t0 < 5000) {
        odrive.clearErrors();
        odrive.setState(AXIS_STATE_CLOSED_LOOP_CONTROL);
        delay(10);
    }
    Serial.println("Closed-loop CONTROL engaged.");

    // Zero the internal position estimate so code’s zero matches ODrive’s zero
    odrive_serial.println("w axis0.encoder.pos_estimate 0");
    delay(20);
    Serial.println("Internal pos_estimate zeroed.");

    // Switch to trapezoidal trajectory input mode
    odrive_serial.println("w axis0.controller.config.input_mode 1");
    delay(100);

    // Store zero reference
    SteeringCommandPosition = 0.0f;
    lastTargetPosition     = 0.0f;
    Serial.print("Steering ZERO set to: ");
    Serial.println(SteeringCommandPosition, 4);

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
    int ch = constrain(channels[3], 350, 1811);
    const int neutral    = 772;
    const int deadband   = 50;
    const float maxAng   = 2.4f;  // radians
    float offsetCmdRad   = 0.0f;
    if (ch > neutral + deadband) {
        offsetCmdRad = (ch - (neutral + deadband))
                     / float(1811 - (neutral + deadband))
                     * maxAng;
    } else if (ch < neutral - deadband) {
        offsetCmdRad = -((neutral - deadband) - ch)
                      / float((neutral - deadband) - 350)
                      * maxAng;
    }

    // Convert radians → turns
    float offsetCmdTurns = offsetCmdRad / Two_pi;

    // Determine target in turns
    if (offsetCmdTurns == 0.0f) {
        target = MEMORY_ZERO;
    } else {
        target = SteeringCommandPosition + offsetCmdTurns;
    }
    lastTargetPosition = target;

    // Send position command (in turns) with velocity limit
    odrive.setPosition(target, VEL_LIMIT);

    long faults = getActiveErrors();
    long reason = getDisarmReason();
    // Print telemetry every 100 ms
    if (millis() - lastPrintTime > 100) {
        ODriveFeedback fb = odrive.getFeedback();
        Serial.print("Tgt(turns):");   Serial.print(lastTargetPosition, 4);
        Serial.print("  Pos(turns):"); Serial.print(fb.pos,          4);
        Serial.print("  CH3:");        Serial.println(ch);
        Serial.printf("Active errors: 0x%lX\n", faults);
        Serial.printf("Disarm reason: 0x%lX\n", reason);

        lastPrintTime = millis();
    }
}

float getTarget() {
    return target;
}
