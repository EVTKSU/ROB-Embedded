#include <EVT_ODriver.hpp>

#include <EVT_SlewRateLimiter.hpp>

#include "TransmitterConstants.hpp"
#include "ConversionConstants.hpp"
#include "ModuleConstants.hpp"
using namespace Constants;


namespace MotorControls {
  ODriver::ODriver() {
    fb = {0.0f, 0.0f};

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

    if (oDrive.getState() != ODriveAxisState::AXIS_STATE_UNDEFINED) {
      getVoltage();
      getCurrent();
      return true;
    }

    return false;
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
    IOConstants::oDriveSerial.println("w axis0.trap_traj.config.vel_limit " + String(ControlConstants::oDriveVelLimit));

    // Configure the acceleration and deceleration limts
    IOConstants::oDriveSerial.println("w axis0.trap_traj.config.accel_limit " + String(ControlConstants::oDriveAccelLimit));
    IOConstants::oDriveSerial.println("w axis0.trap_traj.config.decel_limit " + String(ControlConstants::oDriveAccelLimit));
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


  String ODriver::getInputModeString() {
    switch ((ODriveInputMode)oDrive.getParameterAsInt("axis0.controller.config.input_mode")) {
      case (ODriveInputMode::INPUT_MODE_INACTIVE):
        return "INPUT_MODE_INACTIVE";
      case (ODriveInputMode::INPUT_MODE_PASSTHROUGH):
        return "INPUT_MODE_PASSTHROUGH";
      case (ODriveInputMode::INPUT_MODE_VEL_RAMP):
        return "INPUT_MODE_VEL_RAMP";
      case (ODriveInputMode::INPUT_MODE_POS_FILTER):
        return "INPUT_MODE_POS_FILTER";
      case (ODriveInputMode::INPUT_MODE_MIX_CHANNELS):
        return "INPUT_MODE_MIX_CHANNELS";
      case (ODriveInputMode::INPUT_MODE_TRAP_TRAJ):
        return "INPUT_MODE_TRAP_TRAJ";
      case (ODriveInputMode::INPUT_MODE_TORQUE_RAMP):
        return "INPUT_MODE_TORQUE_RAMP";
      case (ODriveInputMode::INPUT_MODE_MIRROR):
        return "INPUT_MODE_MIRROR";
      case (ODriveInputMode::INPUT_MODE_TUNING):
        return "INPUT_MODE_TUNING";
      default:
        return "";
    }
  }


  void ODriver::initCalibration() {
    Serial.println("Performing initial calibration");

    // Run the ODrive motor calibration 
    oDrive.setState(ODriveAxisState::AXIS_STATE_FULL_CALIBRATION_SEQUENCE);

    while (oDrive.getState() != ODriveAxisState::AXIS_STATE_IDLE) {
      delay(20);
    }

    // Clear the ODrive errors and set into IDLE state 
    oDrive.clearErrors();
    oDrive.setState(ODriveAxisState::AXIS_STATE_IDLE);

    // Give some time to manually center the steering
    Serial.printf(
      "You have %0.1f seconds to center the steering\n",
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
      delay(20);
    }

    if (oDrive.getState() != ODriveAxisState::AXIS_STATE_CLOSED_LOOP_CONTROL) {
      Serial.println("ODrive failed to enter closed loop control after calibration");
      Serial.print("Active errors: ");
      Serial.println((long)getActiveErrors());
      Serial.print("Disarm reason: ");
      Serial.println(getDisarmReason());
      ModuleConstants::stateMachine.setErrorState();
      return;
    }

    Serial.println("ODrive is in closed loop control");

    // Configure and enable trapezoidal trajectory 
    configureTrapTrajLimits();
    Serial.println("Trapezoidal trajectory input mode enabled.");

    // Denote the completion of calibration 
    Serial.println("Completed Initial Calibration"); 

    // Configure the ODrive velocity limit
    IOConstants::oDriveSerial.println("w axis0.controller.config.vel_limit " + String(ControlConstants::oDriveVelLimit));

    // Configure the ODrive gains
    IOConstants::oDriveSerial.println("w axis0.controller.config.pos_gain " + String(ControlConstants::oDrivePosGain));
    IOConstants::oDriveSerial.println("w axis0.controller.config.vel_gain " + String(ControlConstants::oDriveVelGain));
    IOConstants::oDriveSerial.println("w axis0.controller.config.vel_integrator_gain " + String(ControlConstants::oDriveIntegratorGain));

    // Configure the ODrive current limits 
    IOConstants::oDriveSerial.println("w axis0.config.motor.current_soft_max " + String(ControlConstants::oDriveSoftCurrentMax));
    IOConstants::oDriveSerial.println("w axis0.config.motor.current_hard_max " + String(ControlConstants::oDriveHardCurrentMax));

    // Configure the thresholds 
    IOConstants::oDriveSerial.println("w axis0.controller.config.spinout_mechanical_power_threshold " + String(ControlConstants::oDriveMechanicalThreshold));
    IOConstants::oDriveSerial.println("w axis0.controller.config.spinout_electrical_power_threshold " + String(ControlConstants::oDriveElectricalThreshold));

    // Print the current input mode 
    Serial.print("Current Input Mode: ");
    Serial.println(getInputMode());

    // ODrive steering is calibrated and ready at this point.
    getVoltage();
    getCurrent();
    systemInitialized = true;

    delay(2'000);

//    if (!ModuleConstants::dynamicBrake.home()) {
      //Serial.println("Dynamic brake homing failed after ODrive calibration");
    //  ModuleConstants::stateMachine.setErrorState();
  //    return;
    //}

    ModuleConstants::transmitter.reacquire();
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
        static size_t lastCalibrationPrint = 0UL;
        if (Serial && ((millis() - lastCalibrationPrint) >= 1'000UL)) {
          Serial.println("Waiting for calibration switch (SWA)");
          lastCalibrationPrint = millis();
        }

        return;
      }
    }

    // Maps current target and sets deadband
    currentTarget = ModuleConstants::transmitter.getChannelValue<float>(
      Signals::ChannelRC::RIGHT_X,
      [&](float val) -> float {
        if (val >= TransmitterConstants::deadbandBounds[0] && val <= TransmitterConstants::deadbandBounds[1]) {
          return absCenterPos; // Creates a deadband of a 5% to be a zero position
        } else {
          return constrain(
            map(
              val,
              TransmitterConstants::minRC,
              TransmitterConstants::maxRC,
              absCenterPos - ControlConstants::oDriveMaxTurns,
              absCenterPos + ControlConstants::oDriveMaxTurns
            ),
            absCenterPos - ControlConstants::oDriveMaxTurns,
            absCenterPos + ControlConstants::oDriveMaxTurns
          );
        }
      }
    );

    // Send position command (in turns) with a velocity limit
    currentTarget = turnLimiter.calculate(currentTarget);
    fb.pos = currentTarget;
    fb.vel = 0.0f;
    oDrive.trapezoidalMove(currentTarget);
  }


  void ODriver::updateAuto(float steering) {
    // Send position command in turns
    currentTarget = absCenterPos + steering;
    fb.pos = currentTarget;
    fb.vel = 0.0f;
    oDrive.trapezoidalMove(currentTarget);
  }


  void ODriver::sendCommand(String cmd) {
    IOConstants::oDriveSerial.println(cmd);
  }

  
  template <size_t N>
  void ODriver::sendCommands(String (&cmd)[N]) {
    for (size_t i = 0; i < N; i++) {
      IOConstants::oDriveSerial.println(cmd[i]);
    }
  }


  void ODriver::reset() {
    Serial.println("Clearing errors");
    oDrive.clearErrors();
    systemInitialized = false;
  }


  void ODriver::waitUntilState(ODriveAxisState state, size_t timeout) {
    initTime = millis();

    while ((oDrive.getState() != state) && (millis() - initTime < timeout)) {
      delay(20);
    }
  }


  float ODriver::getTarget() {
    return currentTarget;
  }


  float ODriver::turnsToSteeringDegrees(float positionTurns) const {
    return ((positionTurns - absCenterPos) / ControlConstants::oDriveMaxTurns) * ControlConstants::steeringMaxDegrees;
  }


  float ODriver::getTargetDegrees() {
    return turnsToSteeringDegrees(currentTarget);
  }

  
  float ODriver::getVoltage() {
    cachedVoltage = oDrive.getParameterAsFloat("vbus_voltage");
    return cachedVoltage;
  }


  float ODriver::getCurrent() {
    cachedCurrent = oDrive.getParameterAsFloat("ibus");
    return cachedCurrent;
  }


  float ODriver::getCachedVoltage() {
    return cachedVoltage;
  }


  float ODriver::getCachedCurrent() {
    return cachedCurrent;
  }


  bool ODriver::isCalibrated() {
    return systemInitialized;
  }


  ODriveFeedback ODriver::getFeedback() {
    fb = oDrive.getFeedback();
    return fb;
  }


  ODriveFeedback ODriver::getCachedFeedback() {
    return fb;
  }


  float ODriver::getCachedSteeringDegrees() {
    return turnsToSteeringDegrees(fb.pos);
  }


  ODriveAxisState ODriver::getAxisState() {
    return oDrive.getState();
  }
}
