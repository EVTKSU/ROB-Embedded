
#include <SPI.h>
#include <sstream>

#include <EVT_AutoMode.hpp>

#include "ConversionConstants.hpp"
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
// Expected format: "throttle,steering,emergency"
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
        steering = atof(token);
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
    ModuleConstants::vesc.updateAuto(0.0f, 20.0f);
    return;
  }

  if (throttle < 0.0f) {
    brakeCurrent = (throttle * -1.0f) / 5; // brake current = throttle value divided by 5. if max throttle is -100 then max brake current is 20A for now. val can be changed
    brakeState = true;

    ModuleConstants::vesc.updateAuto(0.0f, brakeCurrent);
    return;
  } else if (throttle == 0.0f) {
    coasting = true;  
    ModuleConstants::vesc.updateAuto(0.0f, 0.0f);
    
    return;
  } else if (throttle > 0.0f) {
    throttleRpm = (throttle / 100.0f) * 7500.0f; // map throttle 0-100 to 0 - maxRPM (7500 for old vescrpm,14800 new theoretical vesc)
    brakeState = false;
    coasting = false;

    ModuleConstants::vesc.updateAuto(throttleRpm, 0.0f);

    return;
  }
}


void CtrlOdrive() {
  // Map steering (-100 to +100) to ODrive position range (-maxPos to +maxPos)
  if (steering <-0.25f){
    SteeringPos = (steering / 100.0f) * 2.25f; // map steering -100 to 0 to -maxPos to 0
  } else if (steering > 0.25f) {
    SteeringPos = (steering / 100.0f) * 2.25f; // map steering 0 to +100 to 0 to +maxPos
  } else {
    SteeringPos = 0.0f; // center position
  }
      
  ModuleConstants::odrive.updateAuto(SteeringPos);
}


void updateAutonomousMode() {
  std::string rawCommands = ModuleConstants::ethernet.receiveUDP();

  Serial.print(" | Throttle(rpm): ");
  Serial.print(throttleRpm);
  Serial.print(" | Steering (turns): ");
  Serial.print(SteeringPos);
  Serial.print(" | Emergency: ");
  Serial.println(emergency ? "YES" : "NO");
  Serial.println(brakeState);
  Serial.println(coasting);


  CtrlVesc();
  CtrlOdrive();
  ModuleConstants::ethernet.sendTelemetry(
    ModuleConstants::stateMachine.checkError(),
    ModuleConstants::stateMachine.toString(ModuleConstants::stateMachine.getState()),
    (float)(ModuleConstants::vesc.getState().erpmCommand / ControlConstants::vescPolePairs),
    ModuleConstants::odrive.getFeedback().pos,
    ModuleConstants::odrive.getVoltage(),
    ModuleConstants::vesc.getVoltage(),
    ModuleConstants::odrive.getCurrent(),
    ModuleConstants::vesc.getCurrent(),
    ModuleConstants::odrive.getTarget(),
    ModuleConstants::transmitter.getChannelValue(Signals::ChannelRC::RIGHT_X, false),
    ModuleConstants::transmitter.getChannelValue(Signals::ChannelRC::LEFT_Y, false),
    ModuleConstants::driveEncoder.getPosition()
  );

  if (!rawCommands.empty()) {
    setControls(rawCommands);
  }
}


namespace Signals {
  void AutoDriver::updateAuto(const std::string & udpData) {
    if (!udpData.empty()) { // Update the UDP data if the packet isn't an empty string
      strncpy(udpBuffer, udpData.c_str(), sizeof(udpBuffer) - 1);
      udpBuffer[sizeof(udpBuffer) - 1] = '\0';

      token = strtok(udpBuffer, ",");
      index = 0;

      while (token != nullptr) {
        switch (index) {
          case 0:
            vescERPM = atof(token);
            break;
          case 1:
            steeringAngle = constrain(
              atof(token),
              -ControlConstants::steeringMaxDegrees,
              ControlConstants::steeringMaxDegrees
            );
            break;
          case 2:
            emergencyFlag = (atoi(token) != 0);
            break;
        }

        token = strtok(nullptr, ",");
        index++;
      }


      if (index < numFields) { // Skips updating values if a malformed packet is received
        Serial.printf("Malformed control packet (expected %d fields): ", numFields);
        Serial.println(udpData.c_str());
      } else {
        if (emergencyFlag) { // Stop the VESC and go into error state if an error occurs 
          if (Serial) {
            Serial.println("Emergency Flag encountered");
          }
  
          updateVESC(0.0f, 20.0f);

          ModuleConstants::stateMachine.setErrorState();
        } else { // Update the ODrive and VESC if no error occurs 
          updateODrive(steeringAngle);
          if (vescERPM < 0.0f) {
            brakeCurrent = constrain(
              (-vescERPM / ControlConstants::vescMaxERPM) * ControlConstants::vescMaxBrake,
              ControlConstants::vescMinBrake,
              ControlConstants::vescMaxBrake
            );
            updateVESC(0.0f, brakeCurrent);
          } else {
            brakeCurrent = 0.0f;
            updateVESC(vescERPM, 0.0f);
          }
        }
      }
    } else { // Print a warning to the Serial Monitor if the UDP packet is empty
      if (Serial) {
        Serial.println("Received empty UDP packet");
      }
    }
  }


  void AutoDriver::updateODrive(float steeringAngle) {
    float steeringTurns = 0.0f;

    if (steeringAngle < -0.25f || steeringAngle > 0.25f) {
      steeringTurns = (steeringAngle / ControlConstants::steeringMaxDegrees) * ControlConstants::steeringMaxTurns;
    }

    ModuleConstants::odrive.updateAuto(steeringTurns);
  }


  void AutoDriver::updateVESC(float erpm, float current) {
    ModuleConstants::vesc.updateAuto(
      constrain(erpm, ControlConstants::vescMinERPM, ControlConstants::vescMaxERPM), 
      current
    );
  }
}
