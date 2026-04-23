
#include <SPI.h>
#include <sstream>

#include <EVT_VescDriver.hpp>
#include <EVT_Ethernet.hpp>
#include <EVT_ODriver.hpp>

#include "ControlConstants.hpp"
#include "ModuleConstants.hpp"
using namespace Constants;

// IN AUTO MODE, THERE IS NO REVERSE. REVERSE BRAKES IN THIS CASE.

// Global variables for UDP control data
float erpm = 0.0;
float throttle = 0.0;
float steering = 0.0;
float brakeCurrent = 0.0;
float throttleRpm = 0.0;
float SteeringPos = 0.0;
bool emergency = false;
bool brakeState = false;
bool coasting = false;

// Function to parse UDP data and update control variables
// Expected format: "throttle_rpm,steering_degrees,emergency"
void setControls(const std::string &udpData) {
  // Copy string to modifiable buffer
  char udpCopy[128];
  strncpy(udpCopy, udpData.c_str(), sizeof(udpCopy) - 1);
  udpCopy[sizeof(udpCopy) - 1] = '\0';  // Ensure null termination

  char* token = strtok(udpCopy, ",");
  int index = 0;

  while (token != nullptr) {
    switch (index) {
      case 0:
        throttle = atof(token);
        break;
      case 1:
        steering = constrain(
          atof(token),
          -ControlConstants::steeringMaxDegrees,
          ControlConstants::steeringMaxDegrees
        );
        break;
      case 2:
        emergency = (atoi(token) != 0);
        break;
    }

    index++;
    token = strtok(nullptr, ",");
  }

  if (index < 3) {
    Serial.print("Malformed control packet (expected 3 fields): ");
    Serial.println(udpData.c_str());
  }
}


void CtrlVesc() {
  if (emergency) {
    throttleRpm = 0.0f;
    brakeCurrent = 20.0f;
    brakeState = true;
    coasting = false;

    ModuleConstants::vesc.updateAuto(0.0f, brakeCurrent);
    return;
  }

  if (throttle < 0.0f) {
    throttleRpm = 0.0f;
    brakeCurrent = constrain(
      (throttle * -1.0f) / ControlConstants::vescMaxERPM * ControlConstants::vescMaxBrake,
      ControlConstants::vescMinBrake,
      ControlConstants::vescMaxBrake
    );
    brakeState = true;
    coasting = false;

    ModuleConstants::vesc.updateAuto(0.0f, brakeCurrent);
    return;
  } else if (throttle == 0.0f) {
    throttleRpm = 0.0f;
    brakeCurrent = 0.0f;
    brakeState = false;
    coasting = true;  
    ModuleConstants::vesc.updateAuto(0.0f, 0.0f);
    
    return;
  } else if (throttle > 0.0f) {
    throttleRpm = constrain(throttle, ControlConstants::vescMinERPM, ControlConstants::vescMaxERPM);
    brakeCurrent = 0.0f;
    brakeState = false;
    coasting = false;

    ModuleConstants::vesc.updateAuto(throttleRpm, 0.0f);

    return;
  }
}


void CtrlOdrive() {
  // Map steering degrees to the existing ODrive turns command range.
  if (steering < -0.25f) {
    SteeringPos = (steering / ControlConstants::steeringMaxDegrees) * ControlConstants::steeringMaxTurns;
  } else if (steering > 0.25f) {
    SteeringPos = (steering / ControlConstants::steeringMaxDegrees) * ControlConstants::steeringMaxTurns;
  } else {
    SteeringPos = 0.0f; // center position
  }
      
  ModuleConstants::odrive.updateAuto(SteeringPos);
}


void updateAutonomousMode() {
  std::string rawCommands = ModuleConstants::ethernet.receiveUDP();

  if (!rawCommands.empty()) {
    setControls(rawCommands);
  }

  CtrlVesc();
  CtrlOdrive();

  Serial.print(" | Throttle(rpm): ");
  Serial.print(throttleRpm);
  Serial.print(" steering(deg): ");
  Serial.print(steering);
  Serial.print(" steering(turns): ");
  Serial.print(SteeringPos);
  Serial.print(" | Emergency: ");
  Serial.println(emergency ? "YES" : "NO");
  Serial.println(brakeState);
  Serial.println(coasting);
}
