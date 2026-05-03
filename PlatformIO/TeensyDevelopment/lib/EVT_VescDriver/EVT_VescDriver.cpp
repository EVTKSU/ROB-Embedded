#include <EVT_VescDriver.hpp>

#include "ModuleConstants.hpp"
#include "IOConstants.hpp"
using namespace Constants;


namespace MotorControls {
  namespace {
    float mapRCToRange(uint16_t value, uint16_t inMin, uint16_t inMax, float outMin, float outMax) {
      return outMin + ((float)(value - inMin) * (outMax - outMin) / (float)(inMax - inMin));
    }
  }


  VescDriver::VescDriver(HardwareSerial * vescSerial) { 
    (*vescSerial).begin(IOConstants::vescBaudrate); // Starts the given UART instance at correct Baudrate
    vesc.setSerialPort(vescSerial);                 // Sets UART port being used for communication
  }


  void VescDriver::updateRC(uint16_t throttleChannel, uint16_t brakeChannel) {
    throttleChannel = constrain(throttleChannel, TransmitterConstants::minRC, TransmitterConstants::maxRC);
    brakeChannel = constrain(brakeChannel, TransmitterConstants::minRC, TransmitterConstants::maxRC);

    if (brakeChannel <= TransmitterConstants::deadbandBounds[0]) {
      targetValues.erpmCommand = 0.0f;
      targetValues.currentCommand = 0.0f;
      targetValues.brakeCommand = constrain(
        mapRCToRange(
          brakeChannel,
          TransmitterConstants::minRC,
          TransmitterConstants::deadbandBounds[0],
          ControlConstants::vescMaxBrake,
          ControlConstants::vescMinBrake
        ),
        ControlConstants::vescMinBrake,
        ControlConstants::vescMaxBrake
      );

      rpmLimit.setLastValue(0.0f);
      currentLimit.setLastValue(0.0f);
      vesc.setBrakeCurrent(targetValues.brakeCommand);
      return;
    }

    targetValues.brakeCommand = 0.0f;

    if (throttleChannel <= TransmitterConstants::deadbandBounds[1] && throttleChannel >= TransmitterConstants::deadbandBounds[0]) {
      targetValues.erpmCommand = 0.0f;
      targetValues.currentCommand = 0.0f;

      rpmLimit.setLastValue(0.0f);
      currentLimit.setLastValue(0.0f);

      vesc.setBrakeCurrent(targetValues.brakeCommand);
      vesc.setCurrent(0.0f);
    } else if (throttleChannel <= TransmitterConstants::deadbandBounds[0]) {
      targetValues.erpmCommand = rpmLimit.calculate(constrain(
        mapRCToRange(
          throttleChannel,
          TransmitterConstants::minRC,
          TransmitterConstants::deadbandBounds[0],
          -ControlConstants::vescMaxReverseERPM,
          ControlConstants::vescMinERPM
        ),
        -ControlConstants::vescMaxReverseERPM,
        ControlConstants::vescMinERPM
      ));

      targetValues.currentCommand = currentLimit.calculate(constrain(
        mapRCToRange(
          throttleChannel,
          TransmitterConstants::minRC,
          TransmitterConstants::deadbandBounds[0],
          -ControlConstants::vescMaxReverseCurrent,
          0.0f
        ),
        -ControlConstants::vescMaxReverseCurrent,
        0.0f
      ));

      vesc.setCurrent(targetValues.currentCommand);
    } else if (throttleChannel >= TransmitterConstants::deadbandBounds[1]) {
      targetValues.currentCommand = 0.0f;
      currentLimit.setLastValue(0.0f);
      targetValues.erpmCommand = rpmLimit.calculate(constrain(
        mapRCToRange(
          throttleChannel,
          TransmitterConstants::deadbandBounds[1],
          TransmitterConstants::maxRC,
          ControlConstants::vescMinERPM,
          ControlConstants::vescMaxERPM
        ),
        ControlConstants::vescMinERPM,
        ControlConstants::vescMaxERPM
      ));

      vesc.setRPM(targetValues.erpmCommand);
    }
  }


  void VescDriver::updateAuto(float erpm, float brake) {
    targetValues.erpmCommand = erpm;
    targetValues.brakeCommand = brake;
    targetValues.currentCommand = 0.0f;

    if (brake > 0.0f) {
      vesc.setBrakeCurrent(brake);
      return;
    }

    vesc.setRPM(erpm);
  }


  void VescDriver::updateAutoCurrent(float erpm, float current) {
    targetValues.erpmCommand = erpm;
    targetValues.brakeCommand = 0.0f;
    targetValues.currentCommand = current;

    vesc.setCurrent(current);
  }


  String VescDriver::errorToString(mc_fault_code VESC_ERR) {
    switch (VESC_ERR) {
      case (mc_fault_code::FAULT_CODE_NONE): 
        return "FAULT_CODE_NONE";
      case (mc_fault_code::FAULT_CODE_OVER_VOLTAGE):
        return "FAULT_CODE_OVER_VOLTAGE";
      case (mc_fault_code::FAULT_CODE_UNDER_VOLTAGE):
        return "FAULT_CODE_UNDER_VOLTAGE";
      case (mc_fault_code::FAULT_CODE_DRV):
        return "FAULT_CODE_DRV";
      case (mc_fault_code::FAULT_CODE_ABS_OVER_CURRENT):
        return "FAULT_CODE_ABS_OVER_CURRENT";
      case (mc_fault_code::FAULT_CODE_OVER_TEMP_FET):
        return "FAULT_CODE_OVER_TEMP_FET";
      case (mc_fault_code::FAULT_CODE_OVER_TEMP_MOTOR):
        return "FAULT_CODE_OVER_TEMP_MOTOR";
      case (mc_fault_code::FAULT_CODE_GATE_DRIVER_OVER_VOLTAGE):
        return "FAULT_CODE_GATE_DRIVER_OVER_VOLTAGE";
      case (mc_fault_code::FAULT_CODE_GATE_DRIVER_UNDER_VOLTAGE):
        return "FAULT_CODE_GATE_DRIVER_UNDER_VOLTAGE";
      case (mc_fault_code::FAULT_CODE_MCU_UNDER_VOLTAGE):
        return "FAULT_CODE_MCU_UNDER_VOLTAGE";
      case (mc_fault_code::FAULT_CODE_BOOTING_FROM_WATCHDOG_RESET):
        return "FAULT_CODE_BOOTING_FROM_WATCHDOG_RESET";
      case (mc_fault_code::FAULT_CODE_ENCODER_SPI):
        return "FAULT_CODE_ENCODER_SPI";
      case (mc_fault_code::FAULT_CODE_ENCODER_SINCOS_BELOW_MIN_AMPLITUDE):
        return "FAULT_CODE_ENCODER_SINCOS_BELOW_MIN_AMPLITUDE";
      case (mc_fault_code::FAULT_CODE_ENCODER_SINCOS_ABOVE_MAX_AMPLITUDE):
        return "FAULT_CODE_ENCODER_SINCOS_ABOVE_MAX_AMPLITUDE";
      case (mc_fault_code::FAULT_CODE_FLASH_CORRUPTION):
        return "FAULT_CODE_FLASH_CORRUPTION";
      case (mc_fault_code::FAULT_CODE_HIGH_OFFSET_CURRENT_SENSOR_1):
        return "FAULT_CODE_HIGH_OFFSET_CURRENT_SENSOR_1";
      case (mc_fault_code::FAULT_CODE_HIGH_OFFSET_CURRENT_SENSOR_2):
        return "FAULT_CODE_HIGH_OFFSET_CURRENT_SENSOR_2";
      case (mc_fault_code::FAULT_CODE_HIGH_OFFSET_CURRENT_SENSOR_3):
        return "FAULT_CODE_HIGH_OFFSET_CURRENT_SENSOR_3";
      case (mc_fault_code::FAULT_CODE_UNBALANCED_CURRENTS):
        return "FAULT_CODE_UNBALANCED_CURRENTS";
      case (mc_fault_code::FAULT_CODE_BRK):
        return "FAULT_CODE_BRK";
      case (mc_fault_code::FAULT_CODE_RESOLVER_LOT):
        return "FAULT_CODE_RESOLVER_LOT";
      case (mc_fault_code::FAULT_CODE_RESOLVER_DOS):
        return "FAULT_CODE_RESOLVER_DOS";
      case (mc_fault_code::FAULT_CODE_RESOLVER_LOS):
        return "FAULT_CODE_RESOLVER_LOS";
      case (mc_fault_code::FAULT_CODE_FLASH_CORRUPTION_APP_CFG):
        return "FAULT_CODE_FLASH_CORRUPTION_APP_CFG";
      case (mc_fault_code::FAULT_CODE_FLASH_CORRUPTION_MC_CFG):
        return "FAULT_CODE_FLASH_CORRUPTION_MC_CFG";
      case (mc_fault_code::FAULT_CODE_ENCODER_NO_MAGNET):
        return "FAULT_CODE_ENCODER_NO_MAGNET";
      case (mc_fault_code::FAULT_CODE_ENCODER_MAGNET_TOO_STRONG):
        return "FAULT_CODE_ENCODER_MAGNET_TOO_STRONG";
      case (mc_fault_code::FAULT_CODE_PHASE_FILTER):
        return "FAULT_CODE_PHASE_FILTER";
      default:
        return "NO_KNOWN_FAULTS";
    }
  }

  
  void VescDriver::printState() {
    Serial.printf(
      "ERPM - %.2f\t| Motor Current - %.2f\t| Brake Current - %.2f\n",
      targetValues.erpmCommand,
      targetValues.currentCommand,
      targetValues.brakeCommand
    );
  }


  VescValues VescDriver::getState() {
    return targetValues;
  }


  float VescDriver::getVoltage() {
    return vesc.data.inpVoltage;
  }


  float VescDriver::getCurrent() {
    return vesc.data.avgMotorCurrent;
  }
}
