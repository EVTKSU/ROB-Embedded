#include <EVT_ODriver.hpp>

#include <EVT_SlewRateLimiter.hpp>
using namespace MotorControls;

#include "TransmitterConstants.hpp"
#include "ConversionConstants.hpp"
#include "ControlConstants.hpp"
#include "ModuleConstants.hpp"
#include "IOConstants.hpp"
using namespace Constants;

// HardwareSerial &odrive_serial = Serial6;
// ODriveUART    odrive(odrive_serial);

// float target;                     // creating steering value to command ODrive
// float absCenterPos    = 0.00f;     // ← NEW: manual “zero” reference

// // ——— Constants ———
// const float VEL_LIMIT    = 120.0;    
// const float ACCEL_LIMIT  = 900.0;    
// const float Two_pi       = 2.0f * 3.14159265358979323846f;
// static const float MAX_STEERING_TURNS    = 4.2;  

// const float rateLimit = 50.0;
// SlewRateLimiter limiter = SlewRateLimiter(rateLimit);

// // ——— State ———
// bool          systemInitialized       = false;
// String        odrvDebug;
// float         SteeringCommandPosition = 0.0f;
// float         lastTargetPosition      = 0.0f;
// bool          errorClearFlag          = false;
// unsigned long lastPrintTime           = 0;


// /**
//  * @brief Configures the ODrive absolute reference position
//  * 
//  * @param absPos Reference position 
//  */
// void configureAbsoluteReference(float absPos) {
//     odrive_serial.println("w axis0.pos_vel_mapper.config.offset "       + String(absPos, 4));
//     delay(20);

//     odrive_serial.println("w axis0.pos_vel_mapper.config.offset_valid true");
//     delay(20);
    
//     odrive_serial.println("w axis0.pos_vel_mapper.config.approx_init_pos "       + String(absPos, 4));
//     delay(20);
    
//     odrive_serial.println("w axis0.pos_vel_mapper.config.approx_init_pos_valid true");
//     delay(20);
//     odrive_serial.println("w axis0.controller.config.absolute_setpoints true");
//     delay(20);
// }


// /**
//  * @brief Configures trapezoidial trajectory limits 
//  */
// void configureTrapTrajLimits() {
//     Serial.println("Setting trap-traj vel/accel limits...");

// }

// /**
//  * @brief Gets the active error codes from the ODrive serial 
//  * 
//  * @return Error codes as an int 
//  */
// int getActiveErrors() {
//     // Path exactly as exposed in the firmware for active errors :contentReference[oaicite:0]{index=0}
//     long errs = odrive.getParameterAsInt("axis0.active_errors");
//     return (long)errs;
// }


// /**
//  * @brief Gets the disarm reason from the Odrive serial
//  * 
//  * @return Disarm reason as an int 
//  */
// int getDisarmReason() {
//     // Path exactly as exposed in the firmware for disarm reason :contentReference[oaicite:0]{index=0}
//     long reason = odrive.getParameterAsInt("axis0.disarm_reason");
//     return (long)reason;
// }


// /**
//  * @brief Gets the current input mode from the ODrive serial
//  * 
//  * @return Current input mode as an int 
//  */
// int getInputMode() {
//     // Path exactly as exposed in the firmware for input mode :contentReference[oaicite:0]{index=0}
//     long input_mode = odrive.getParameterAsInt("axis0.controller.config.input_mode");
//     return (long)input_mode;
// }


// /**
//  * @brief Runs the ODrive calibration sequence 
//  */
// void initCalibration() {
//     Serial.println("InitCalibration ▶ SBUS ch5");
    
//     odrive.setState(AXIS_STATE_MOTOR_CALIBRATION); // Set to calibration state
//     delay(4000);

//     odrive.clearErrors();
//     odrive.setState(AXIS_STATE_ENCODER_OFFSET_CALIBRATION); // Clear any errors and set encoder calibration state
//     delay(4000);

//     Serial.println("Motor & encoder offset calibration complete."); // Mark the end of calibration

//     odrive.setState(AXIS_STATE_IDLE); // Set the ODrive to idle state 
//     Serial.println("Axis set to IDLE. you have 8 seconds to manually home the motor.");
//     delay(8000);
    
//     ODriveFeedback fb = odrive.getFeedback();
//     float absPos = fb.pos;

//     Serial.print("Encoder Position: ");
//     Serial.println(absPos, 4);
//     delay(3000);

//     absCenterPos = absPos; // Store the absolute position as the center reference

//     Serial.print("Center Position: ");
//     Serial.println(absCenterPos, 4);

//     // Set the ODrive to closed loop control 
//     unsigned long t0 = millis();
//     while (odrive.getState() != AXIS_STATE_CLOSED_LOOP_CONTROL && millis() - t0 < 5000) {
//         odrive.clearErrors();
//         odrive.setState(AXIS_STATE_CLOSED_LOOP_CONTROL);
//         delay(10);
//     }
//    // const int axis = 0;  // steering axis

//     Serial.println("Closed-loop CONTROL engaged.");

//     configureTrapTrajLimits(); // sets velocity and acceleration limits
//     Serial.println("Trapezoidal trajectory input mode enabled.");

//     // Store zero reference
//     delay(1000);
//     int current_mode = odrive.getParameterAsInt("axis0.controller.config.input_mode");

   
//     delay(1000);

//     Serial.println("initCalibration ▶ complete"); // Denote the completion of calibration 
//  // SETTING ALL PARAMETERS BELOW:   
// odrive_serial.println("w axis0.controller.config.vel_limit 125");
// Serial.println("set vel_limit to 125");
// odrive_serial.println("w axis0.controller.config.pos_gain 100");
// Serial.println("set pos_gain to 100");
// odrive_serial.println("w axis0.controller.config.vel_gain 0.072");
// Serial.println("set vel_gain to 0.072");
// odrive_serial.println("w axis0.controller.config.vel_integrator_gain 0");
// odrive_serial.println("w axis0.controller.config.vel_limit " + String(VEL_LIMIT));
// Serial.println("set vel_limit to " + String(VEL_LIMIT));
// odrive_serial.println("w axis0.trap_traj.config.vel_limit " + String(VEL_LIMIT));
// Serial.println("set trap_traj vel_limit to " + String(VEL_LIMIT));
// odrive_serial.println("w axis0.trap_traj.config.accel_limit " + String(ACCEL_LIMIT));
// odrive_serial.println("w axis0.trap_traj.config.decel_limit " + String(ACCEL_LIMIT));

//  Serial.print("Current input_mode = ");
//     Serial.println(current_mode);
// }


// /**
//  * @brief Start serial communication to the Teensy and ODrive
//  */
// void setupOdrv() {
//     Serial.begin(115200);
//     odrive_serial.begin(115200);

//     Serial.println("ODrive serial init...");
//     unsigned long t0 = millis();
//     while (odrive.getState() == AXIS_STATE_UNDEFINED && millis() - t0 < 15000) {
//         delay(50);
//     }

//     Serial.println(odrive.getState() == AXIS_STATE_UNDEFINED
//                    ? "ODrive not found, proceeding standalone."
//                    : "ODrive detected.");
//     Serial.println("Awaiting SBUS ch5 to initCalibration.");
// }


// /**
//  * @brief Control loop used to run the ODrive
//  */
// void updateOdrvControl() {
//     // LED heartbeat until init
//     static unsigned long ledT = 0;
//     if (!systemInitialized && millis() - ledT > 500) {
//         ledT = millis();
//         digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN));
//     } else if (systemInitialized) {
//         digitalWrite(STATUS_LED_PIN, HIGH);
//     }


//     // Clear-errors on SBUS ch4
//     if ( > 1500 && !errorClearFlag) {
//         errorClearFlag     = true;
//         Serial.println("SBUS4 ▶ clearErrors()");
//         odrive.clearErrors();
//         systemInitialized = false;
//     }

//     if (ch4 < 1500) {
//         errorClearFlag = false;
//     }


//     // Trigger calibration/homing on ch5
//     if (!systemInitialized) {
//         if (channels[5] > 900) {
//             initCalibration();
//             systemInitialized = true;
//         } else {
//             Serial.print("Waiting ch5>900 ▶ ");
//             Serial.println(channels[5]);
//             return;
//         }
//     }

    
//     // SBUS ch3 → offsetCmd (deadband + mapping), in radians
//     int ch = constrain(channels[3], 377, 1763);
//     const int neutral    = 1075;
//     const int deadband   = 50;

//     if (ch > (neutral + deadband)) {
//         // Map [NEUTRAL+DEADBAND … SBUS_MAX] → [0 … +MAX_STEERING_TURNS]
//         float fractionPos = float(ch - (neutral + deadband))
//                           / float(1811 - (neutral + deadband));
//         float steeringTurns = fractionPos * MAX_STEERING_TURNS;
//         target = absCenterPos + steeringTurns;
//     } else if (ch < (neutral - deadband)) {
//         // Map [NEUTRAL‑DEADBAND … SBUS_MIN] → [0 … -MAX_STEERING_TURNS]
//         float fractionNeg = float((neutral - deadband) - ch)
//                           / float((neutral - deadband) - 350);
//         float steeringTurns = fractionNeg * MAX_STEERING_TURNS;
//         target = absCenterPos - steeringTurns;
//     } else {
//         // Within deadband: hold exactly at absCenterPos
//         target = absCenterPos;
//     }
    

//     // Send position command (in turns) with velocity limit
//     odrive.trapezoidalMove(limiter.calculate(target));

//     // long faults = getActiveErrors();
//     // long reason = getDisarmReason();


//     //     lastPrintTime = millis();
//     // }
// }


// /**
//  * @brief Gets the current target 
//  * 
//  * @return The current target for the ODrive 
//  */
// float getTarget() {
//   return target;
// }


namespace MotorControls {
  ODriver::ODriver() {
    // Start the serial monitor if it hasn't already been started 
    if (!Serial) {
      Serial.begin(IOConstants::serialBaudrate);
    }

    // Begin ODrive serial communication 
    IOConstants::oDriveSerial.begin(IOConstants::oDriveBaudrate);
  }
  
  
  bool ODriver::setup() {
    Serial.println("ODrive Serial initializing");
  
    // Wait for the ODrive to be defined or for 15 seconds
    unsigned long t0 = millis();
    while ((oDrive.getState() == ODriveAxisState::AXIS_STATE_UNDEFINED) && (millis() - t0 < 15'000)) {
      delay(50);
    }
  
    // Print serial output
    Serial.println(
      oDrive.getState() == ODriveAxisState::AXIS_STATE_UNDEFINED ? 
      "ODrive not found, proceeding without it" :
      "ODrive found"
    );

    return oDrive.getState() != ODriveAxisState::AXIS_STATE_UNDEFINED;
  }


  void ODriver::configureAbsoluteReference(float pos) {
    IOConstants::oDriveSerial.println("w axis0.pos_vel_mapper.config.offset " + String(pos, 4));
    delay(20);

    IOConstants::oDriveSerial.println("w axis0.pos_vel_mapper.config.offset_valid true");
    delay(20);
    
    IOConstants::oDriveSerial.println("w axis0.pos_vel_mapper.config.approx_init_pos " + String(pos, 4));
    delay(20);
    
    IOConstants::oDriveSerial.println("w axis0.pos_vel_mapper.config.approx_init_pos_valid true");
    delay(20);

    IOConstants::oDriveSerial.println("w axis0.controller.config.absolute_setpoints true");
    delay(20);
  }


  void ODriver::configureTrapTrajLimits() {
    Serial.println("Configuring trapezoidal trajectory limits");

    // Configure the trapezoidal velocity limit
    IOConstants::oDriveSerial.println("w axis0.trap_traj.config.vel_limit " + String(velLimit));

    // Configure the acceleration and deceleration limts
    IOConstants::oDriveSerial.println("w axis0.trap_traj.config.accel_limit " + String(accelLimit));
    IOConstants::oDriveSerial.println("w axis0.trap_traj.config.decel_limit " + String(accelLimit));
  }
  

  ODriveError ODriver::getActiveErrors() {
    ODriveError error = (ODriveError)oDrive.getParameterAsInt("axis0.active_errors");
    return error;
 }


  long ODriver::getDisarmReason() {
    long reason = oDrive.getParameterAsInt("axis0.disarm_reason");
    return reason;
  }


  ODriveInputMode ODriver::getInputMode() {
    ODriveInputMode input_mode = (ODriveInputMode)oDrive.getParameterAsInt("axis0.controller.config.input_mode");
    return input_mode;
  }


  void ODriver::initCalibration() {
    Serial.println("Performing initial calibration");

    // Run the ODrive motor calibration 
    oDrive.setState(ODriveAxisState::AXIS_STATE_MOTOR_CALIBRATION);
    delay(4'000); 

    // Runs the ODrive encoder position calibration 
    oDrive.setState(ODriveAxisState::AXIS_STATE_ENCODER_OFFSET_CALIBRATION);
    delay(4'000);

    // Clear the ODrive errors and set into IDLE state 
    oDrive.clearErrors();
    oDrive.setState(ODriveAxisState::AXIS_STATE_IDLE);

    // Give some time to manually center the steering
    Serial.printf(
      "You have %f seconds to center the steering\n",
      steeringCenterTime
    );

    delay(ConversionConstants::secToMillis * steeringCenterTime); 

    // Record the center position 
    fb = oDrive.getFeedback();
    absCenterPos = fb.pos;

    // Print the center position 
    Serial.print("Center Position: ");
    Serial.println(absCenterPos, 4);

    // Enable closed loop control 
    initTime = millis();
    while ((oDrive.getState() != ODriveAxisState::AXIS_STATE_CLOSED_LOOP_CONTROL) && (millis() - initTime < 5'000)) {
      oDrive.clearErrors();
      oDrive.setState(ODriveAxisState::AXIS_STATE_CLOSED_LOOP_CONTROL);
      delay(5'000);
    }

    Serial.println("Starting closed loop control");

    // Configure and enable trapezoidal trajectory 
    configureTrapTrajLimits();
    Serial.println("Trapezoidal trajectory input mode enabled.");

    // Denote the completion of calibration 
    Serial.println("Completed Initial Calibration"); 

    // Configure the ODrive velocity limit
    IOConstants::oDriveSerial.println("w axis0.controller.config.vel_limit " + String(velLimit));

    // Configure the ODrive gains
    IOConstants::oDriveSerial.println("w axis0.controller.config.pos_gain " + String(positionGain));
    IOConstants::oDriveSerial.println("w axis0.controller.config.vel_gain " + String(velocityGain));
    IOConstants::oDriveSerial.println("w axis0.controller.config.vel_integrator_gain " + String(integratorGain));

    // Configure the ODrive current limits 
    IOConstants::oDriveSerial.println("w axis0.config.motor.current_soft_max " + String(softMaxCurrent));
    IOConstants::oDriveSerial.println("w axis0.config.motor.current_hard_max " + String(hardMaxCurrent));

    // Print the current input mode 
    Serial.print("Current Input Mode: ");
    Serial.println(getInputMode());

    delay(2'000);

    systemInitialized = true;
  }

  
  void ODriver::updateRC() {
    ModuleConstants::transmitter.update();

    // LED heartbeat until system is initialized 
    if (!systemInitialized && (millis() - initTime) > 500) { 
      initTime = millis();
      digitalWrite(IOConstants::ledBuiltIn, !digitalRead(IOConstants::ledBuiltIn));
    } else if (systemInitialized) {
      digitalWrite(IOConstants::ledBuiltIn, HIGH);
    }


    // Wait for the calibration switch to be pulled if the system isn't initialized
    if (!systemInitialized) {
      if (ModuleConstants::transmitter.getChannelValue(Signals::ChannelRC::SWA, Signals::ControlRC::mapSwitches)) {
        initCalibration();
      } else {
        Serial.println("Waiting for calibration switch (SWA)");

        return;
      }
    }


    // Clear the active ODrive errors when SWH is pulled
    // if (ModuleConstants::transmitter.getChannelValue(Signals::ChannelRC::SWH, Signals::ControlRC::mapSwitches) && !errorClearFlag) {
    //   errorClearFlag = true;
    //   Serial.println("Clearing errors");
    //   oDrive.clearErrors();
    //   systemInitialized = false;
    // }

    // if (ModuleConstants::transmitter.getChannelValue(Signals::ChannelRC::SWH, Signals::ControlRC::mapSwitches)) {
    //   errorClearFlag = false;
    // }


    // Maps current target and sets deadband
    currentTarget = ModuleConstants::transmitter.getChannelValue<float>(
      Signals::ChannelRC::RIGHT_X,
      [&](float val) -> float {
        if (val >= TransmitterConstants::midRC - 40 && val <= TransmitterConstants::midRC + 40) {
          Serial.println("In deadband");
          return absCenterPos; // Creates a deadband of a 5% to be a zero position
        } else {
          Serial.println("Running");
          return constrain(
            map(
              val,
              TransmitterConstants::minRC,
              TransmitterConstants::maxRC,
              absCenterPos - maxTurns,
              absCenterPos + maxTurns
            ),
            absCenterPos - maxTurns,
            absCenterPos + maxTurns
          );
        }
      }
    );

    Serial.printf(
      "Target: %f\t| RC: %d\n\n",
      currentTarget,
      ModuleConstants::transmitter.getChannelValue(Signals::ChannelRC::RIGHT_X, false)
    );

    // Send position command (in turns) with a velocity limit
    oDrive.trapezoidalMove(currentTarget);
  }


  void ODriver::updateAuto(float steering) {
    // Send position command (in turns) with a velocity limit
    oDrive.trapezoidalMove(turnLimiter.calculate(steering));
  }


  void ODriver::sendCommand(String cmd) {
    IOConstants::oDriveSerial.println(cmd);
  }


  void ODriver::sendCommands(String cmd[]) {
    for (int i = 0; i < (sizeof(cmd) / sizeof(cmd[0])); i++) {
      IOConstants::oDriveSerial.println(cmd[i]);
    }
  }


  float ODriver::getTarget() {
    return currentTarget;
  }


  bool ODriver::isCalibrated() {
    return systemInitialized;
  }
}