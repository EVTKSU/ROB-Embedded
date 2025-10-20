#include "EVT_ODriver.h"
#include "EVT_RC.hpp" // for channels[]

#include <EVT_SlewRateLimiter.hpp>

HardwareSerial &odrive_serial = Serial6;
ODriveUART odrive(odrive_serial);

float target;
float absCenterPos = 0.00f;

// ——— Constants ———

const float VEL_LIMIT    = 24.0f;    
const float ACCEL_LIMIT  = 10.0f;    
const float Two_pi       = 2.0f * 3.14159265358979323846f;
static const float MAX_STEERING_TURNS = 2.75;  

const int neutral    = 1075;
const int deadband   = 50;

const float rateLimit = VEL_LIMIT;
SlewRateLimiter limiter = SlewRateLimiter(rateLimit); // Rate limiter for the steering value


// ——— State ———

bool          systemInitialized       = false;
String        odrvDebug;
float         SteeringCommandPosition = 0.0f;  // Current steering position 
float         lastTargetPosition      = 0.0f;  // Previous steering positon 
bool          errorClearFlag          = false; // Condition to clear ODrive errors 
unsigned long lastPrintTime           = 0;     // Last time value of debug messages 

int ch3; // RC Transmitter channel 3
int ch4; // RC Transmitter channel 4

ODriveFeedback fb;
static unsigned long ledT;
unsigned long t0;
int current_mode; // Current ODrive input mode

float absPos;
float fractionPos;
float steeringTurns;
float fractionNeg;


/**
 * @brief Configures the ODrive absolute reference position
 * 
 * @param absPos Reference position 
 */
void configureAbsoluteReference(float absPos) {
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


/**
 * @brief Configures trapezoidial trajectory limits 
 */
void configureTrapTrajLimits() {
    Serial.println("Setting trap-traj vel/accel limits...");

    odrive_serial.println("w axis0.controller.config.vel_limit " + String(VEL_LIMIT));
    delay(20);

    odrive_serial.println("w axis0.controller.config.accel_limit " + String(ACCEL_LIMIT));
    delay(20);
}


/**
 * @brief Gets the active error codes from the ODrive serial 
 * 
 * @return Error codes as an int 
 */
int getActiveErrors() {
    // Path exactly as exposed in the firmware for active errors :contentReference[oaicite:0]{index=0}
    return odrive.getParameterAsInt("axis0.active_errors");
}


/**
 * @brief Gets the disarm reason from the Odrive serial
 * 
 * @return Disarm reason as an int 
 */
int getDisarmReason() {
    // Path exactly as exposed in the firmware for disarm reason :contentReference[oaicite:0]{index=0}
    return odrive.getParameterAsInt("axis0.disarm_reason");
}


/**
 * @brief Gets the current input mode from the ODrive serial
 * 
 * @return Current input mode as an int 
 */
int getInputMode() {
    // Path exactly as exposed in the firmware for input mode :contentReference[oaicite:0]{index=0}
    return odrive.getParameterAsInt("axis0.controller.config.input_mode");
}


/**
 * @brief Runs the ODrive calibration sequence 
 */
void initCalibration() {
    Serial.println("InitCalibration ▶ SBUS ch5");
    
    odrive.setState(ODriveAxisState::AXIS_STATE_MOTOR_CALIBRATION); // Set to calibration state
    delay(4000);

    odrive.clearErrors();
    odrive.setState(ODriveAxisState::AXIS_STATE_ENCODER_OFFSET_CALIBRATION); // Clear any errors and set encoder calibration state
    delay(4000);

    Serial.println("Motor & encoder offset calibration complete."); // Mark the end of calibration

    odrive.setState(ODriveAxisState::AXIS_STATE_IDLE); // Set the ODrive to idle state 
    Serial.println("Axis set to IDLE. you have 8 seconds to manually home the motor.");
    delay(8000);
    
    fb = odrive.getFeedback();
    absPos = fb.pos;

    Serial.print("Encoder Position: ");
    Serial.println(absPos, 4);
    delay(3000);

    absCenterPos = absPos; // Store the absolute position as the center reference

    Serial.print("Center Position: ");
    Serial.println(absCenterPos, 4);

    // Set the ODrive to closed loop control 
    t0 = millis();
    while (odrive.getState() != ODriveAxisState::AXIS_STATE_CLOSED_LOOP_CONTROL && millis() - t0 < 5000) {
        odrive.clearErrors();
        odrive.setState(ODriveAxisState::AXIS_STATE_CLOSED_LOOP_CONTROL);
        delay(10);
    }

    Serial.println("Closed-loop CONTROL engaged.");

    // Switch to pass-through input mode
    odrive_serial.println("w axis0.controller.config.input_mode 1");
    delay(100);

    configureTrapTrajLimits(); // sets velocity and acceleration limits
    Serial.println("Trapezoidal trajectory input mode enabled.");

    // Store zero reference
    delay(1000);
    current_mode = getInputMode();

    Serial.print("Current input_mode = ");
    Serial.println(current_mode);
    delay(1000);

    Serial.println("initCalibration ▶ complete"); // Denote the completion of calibration 
}


void setupOdrv() {
    Serial.begin(115200);
    odrive_serial.begin(115200);

    Serial.println("ODrive serial init...");

    t0 = millis();
    while (odrive.getState() == ODriveAxisState::AXIS_STATE_UNDEFINED && millis() - t0 < 15000) {
        delay(50);
    }

    Serial.println(odrive.getState() == ODriveAxisState::AXIS_STATE_UNDEFINED
                   ? "ODrive not found, proceeding standalone."
                   : "ODrive detected.");
    Serial.println("Awaiting SBUS ch5 to initCalibration.");
}


void updateOdrvControl() {
    // LED heartbeat until init
    ledT = 0;
    if (!systemInitialized && millis() - ledT > 500) {
        ledT = millis();
        digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN));
    } else if (systemInitialized) {
        digitalWrite(STATUS_LED_PIN, HIGH);
    }


    // Clear-errors on SBUS ch4
    ch4 = channels[4];
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
    ch3 = constrain(channels[3], 377, 1763);

    if (ch3 > (neutral + deadband)) {
        // Map [NEUTRAL+DEADBAND … SBUS_MAX] → [0 … +MAX_STEERING_TURNS]
        fractionPos = float(ch3 - (neutral + deadband))
                          / float(1811 - (neutral + deadband));
        steeringTurns = fractionPos * MAX_STEERING_TURNS;
        target = absCenterPos + steeringTurns;
    } else if (ch3 < (neutral - deadband)) {
        // Map [NEUTRAL‑DEADBAND … SBUS_MIN] → [0 … -MAX_STEERING_TURNS]
        fractionNeg = float((neutral - deadband) - ch3)
                          / float((neutral - deadband) - 350);
        steeringTurns = fractionNeg * MAX_STEERING_TURNS;
        target = absCenterPos - steeringTurns;
    } else {
        // Within deadband: hold exactly at absCenterPos
        target = absCenterPos;
    }


    // Send position command (in turns) with velocity limit
    odrive.setPosition(limiter.calculate(target)); // 0.0f for no torque feedforward
}


float getTarget() {
    return target;
}
