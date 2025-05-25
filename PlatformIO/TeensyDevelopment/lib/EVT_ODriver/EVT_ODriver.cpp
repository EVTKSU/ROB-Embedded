#include "EVT_ODriver.h"
#include "EVT_RC.h"          // for channels[]

HardwareSerial &odrive_serial = Serial6;
ODriveUART    odrive(odrive_serial);

// ——— Constants ———
float MEMORY_ZERO = -1.77f;  // hard coded zero position
const float          VEL_LIMIT   = 5.0f;     // rad/s
const float          ACCEL_LIMIT = 1.0f;     // rad/s²

// ——— State ———
bool   systemInitialized   = false;
String odrvDebug;
float  SteeringCommandPosition  = MEMORY_ZERO;
float  lastTargetPosition  = MEMORY_ZERO;
bool   errorClearFlag      = false;
unsigned long lastPrintTime = 0;

// ——— Helpers ———
void configureAbsoluteReference(float absPos) {
    Serial.print("Configuring ABS ref at: ");
    Serial.println(absPos, 4);
    odrive_serial.println("w axis0.pos_vel_mapper.config.offset " + String(absPos, 4));
    delay(20);
    odrive_serial.println("w axis0.pos_vel_mapper.config.offset_valid true");
    delay(20);
    odrive_serial.println("w axis0.pos_vel_mapper.config.approx_init_pos " + String(absPos, 4));
    delay(20);
    odrive_serial.println("w axis0.pos_vel_mapper.config.approx_init_pos_valid true");
    delay(20);
    odrive_serial.println("w axis0.controller.config.absolute_setpoints true");
    delay(20);
}

void configureTrapTrajLimits() {  // this sets the velocity and acceleration limits for the trapezoidal trajectory
    Serial.println("Setting trap-traj vel/accel limits...");
    odrive_serial.println("w axis0.trap_traj.config.vel_limit "   + String(VEL_LIMIT));
    delay(20);
    odrive_serial.println("w axis0.trap_traj.config.accel_limit " + String(ACCEL_LIMIT));
    delay(20);
}

// ——— Calibration & Homing ———
void initCalibration() {
    Serial.println("InitCalibration ▶ SBUS ch5");
    ODriveFeedback fb = odrive.getFeedback();
    float absPos = fb.pos;
    Serial.print("pre‑cal abs pos: ");
    Serial.println(absPos, 4);
    delay(3000);
    // Motor & encoder offset calibration (does this in real life you see it move)
    odrive.setState(AXIS_STATE_MOTOR_CALIBRATION);
    delay(4000);
    odrive.clearErrors();
    odrive.setState(AXIS_STATE_ENCODER_OFFSET_CALIBRATION);
    delay(4000);

    

    // Configure ABS reference frame
    configureAbsoluteReference(absPos); 
    //not sure if this does anything yet but im too afraid to remove it

    // Velocity/accel caps
    configureTrapTrajLimits();

    // Engage closed-loop & TRAP_TRAJ input
    unsigned long t0 = millis();
    while (odrive.getState() != AXIS_STATE_CLOSED_LOOP_CONTROL && millis() - t0 < 5000) {
        odrive.clearErrors();
        odrive.setState(AXIS_STATE_CLOSED_LOOP_CONTROL);
        delay(10);
    }
    Serial.println("Closed-loop CONTROL engaged.");
    odrive_serial.println("w axis0.controller.config.input_mode 1");
    delay(100);

    // Store zero reference
    SteeringCommandPosition = absPos;
    lastTargetPosition = absPos;
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

    // Clear‑errors on SBUS ch4
    int ch4 = channels[4];
    if (ch4 > 1500 && !errorClearFlag) {
        errorClearFlag = true;
        Serial.println("SBUS4 ▶ clearErrors()");
        odrive.clearErrors();
        systemInitialized = false;
    }
    if (ch4 < 1500) errorClearFlag = false;


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

    // SBUS ch3 → offsetCmd (deadband + mapping)
    int ch = constrain(channels[3], 350, 1811);  // 350-1811 is the range of the RC channel
    const int neutral  = 772, deadband = 50; // neutral is the center position (sbus val 772), deadband is the deadzone around it
    const float maxAng = 2.4f;
    float offsetCmd = 0; // offset command is the joystick value mapped to radians
    if      (ch > neutral + deadband)
        offsetCmd = (ch - (neutral+deadband)) / float(1811-(neutral+deadband)) * maxAng;
    else if (ch < neutral - deadband)
        offsetCmd = -((neutral-deadband)-ch) / float((neutral-deadband)-350) * maxAng;
    // else offsetCmd = 0 → hold zero
    else { // if within deadband, hold zero
        offsetCmd = 0;
    }

    float target; // creating steering value to command ODrive
    if (offsetCmd == 0.0f) {
        target = MEMORY_ZERO; // memory zero is hard coded zero position so it goes there
    } else {
        target = SteeringCommandPosition + offsetCmd; // when joystiick moved it controls the steering
    }
    lastTargetPosition = target;
    odrive.setPosition(target, VEL_LIMIT); // sets target position with velocity limit so we dont blow shit up
    
    // Print telemetry every 100 ms
    if (millis() - lastPrintTime > 100) {
        ODriveFeedback fb = odrive.getFeedback();
        Serial.print("Tgt:"); Serial.print(lastTargetPosition,2);
        Serial.print("  Pos:"); Serial.print(fb.pos,2);
        Serial.print("  CH3:"); Serial.println(ch);
        lastPrintTime = millis();
    }
}