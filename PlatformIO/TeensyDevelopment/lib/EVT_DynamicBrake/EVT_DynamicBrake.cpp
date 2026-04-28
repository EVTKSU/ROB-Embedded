#include <EVT_DynamicBrake.hpp>

#include "ControlConstants.hpp"
#include "IOConstants.hpp"
using namespace Constants;


namespace MotorControls {
  void DynamicBrake::setup() {
    pinMode(IOConstants::dynamicBrakePulsePin, OUTPUT);
    pinMode(IOConstants::dynamicBrakeDirPin, OUTPUT);
    pinMode(IOConstants::dynamicBrakeLimitSwitchPin, INPUT_PULLUP);

    digitalWrite(IOConstants::dynamicBrakePulsePin, LOW);
    digitalWrite(IOConstants::dynamicBrakeDirPin, LOW);
  }


  void DynamicBrake::pulseStep() {
    digitalWrite(IOConstants::dynamicBrakePulsePin, HIGH);
    delayMicroseconds(ControlConstants::dynamicBrakeStepPulseUs);
    digitalWrite(IOConstants::dynamicBrakePulsePin, LOW);
    delayMicroseconds(ControlConstants::dynamicBrakeStepDelayUs);
  }


  void DynamicBrake::stepMotor(int steps, bool directionForward) {
    if (steps <= 0) {
      return;
    }

    const bool dirLevel = directionForward
      ? ControlConstants::dynamicBrakeForwardDirLevel
      : !ControlConstants::dynamicBrakeForwardDirLevel;

    digitalWrite(IOConstants::dynamicBrakeDirPin, dirLevel ? HIGH : LOW);
    delayMicroseconds(ControlConstants::dynamicBrakeDirSetupUs);

    for (int i = 0; i < steps; i++) {
      if (!directionForward && isLimitHit()) {
        currentPositionSteps = 0;
        break;
      }

      pulseStep();
      currentPositionSteps += directionForward ? 1 : -1;

      if (currentPositionSteps < 0) {
        currentPositionSteps = 0;
      }
    }
  }


  bool DynamicBrake::isLimitHit() const {
    const int state = digitalRead(IOConstants::dynamicBrakeLimitSwitchPin);
    return ControlConstants::dynamicBrakeLimitActiveLow ? state == LOW : state == HIGH;
  }


  bool DynamicBrake::home() {
    digitalWrite(
      IOConstants::dynamicBrakeDirPin,
      ControlConstants::dynamicBrakeForwardDirLevel ? LOW : HIGH
    );
    delayMicroseconds(ControlConstants::dynamicBrakeDirSetupUs);

    for (int i = 0; i < ControlConstants::dynamicBrakeHomeMaxSteps; i++) {
      if (isLimitHit()) {
        currentPositionSteps = 0;
        stepMotor(ControlConstants::dynamicBrakePedalOffsetSteps, true);
        homed = true;
        return true;
      }

      pulseStep();
    }

    homed = false;
    return false;
  }


  bool DynamicBrake::update(float brakePercent) {
    brakePercent = constrain(brakePercent, 0.0f, 1.0f);

    if (!homed && !home()) {
      if (Serial) {
        Serial.println("Dynamic brake failed to home");
      }

      return false;
    }

    const int targetSteps = ControlConstants::dynamicBrakePedalOffsetSteps
      + static_cast<int>(
        brakePercent * (
          ControlConstants::dynamicBrakeMaxSteps
          - ControlConstants::dynamicBrakePedalOffsetSteps
        )
      );
    const int deltaSteps = targetSteps - currentPositionSteps;

    if (deltaSteps > 0) {
      stepMotor(deltaSteps, true);
    } else if (deltaSteps < 0) {
      stepMotor(-deltaSteps, false);
    }

    return true;
  }
}
