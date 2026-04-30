#include <EVT_VescDriver.hpp>

#include "ModuleConstants.hpp"
#include "IOConstants.hpp"
using namespace Constants;


namespace MotorControls {
  VescDriver::VescDriver(HardwareSerial * vescSerial) { 
    (*vescSerial).begin(IOConstants::vescBaudrate); // Starts the given UART instance at correct Baudrate
    vesc.setSerialPort(vescSerial);                 // Sets UART port being used for communication
  }


  void VescDriver::updateRC(uint16_t throttleChannel) {
    if (throttleChannel <= TransmitterConstants::deadbandBounds[1] && throttleChannel >= TransmitterConstants::deadbandBounds[0]) {
      targetValues.brakeCommand = 0.0f;
      targetValues.erpmCommand = 0.0f;

      rpmLimit.setLastValue(0.0f);

      vesc.setBrakeCurrent(targetValues.brakeCommand);
      vesc.setCurrent(0.0f);
    } else if (throttleChannel <= TransmitterConstants::deadbandBounds[0] && targetValues.erpmCommand == 0) {
      targetValues.brakeCommand = map(
        throttleChannel,
        TransmitterConstants::minRC,
        TransmitterConstants::midRC,
        ControlConstants::vescMaxBrake,
        ControlConstants::vescMinBrake
      );

      vesc.setBrakeCurrent(targetValues.brakeCommand);
    } else if (throttleChannel >= TransmitterConstants::deadbandBounds[1] && targetValues.brakeCommand == 0) {
      targetValues.erpmCommand = constrain(
        rpmLimit.calculate(map(
          throttleChannel,
          TransmitterConstants::midRC,
          TransmitterConstants::maxRC,
          ControlConstants::vescMinERPM,
          ControlConstants::vescMaxERPM
        )),
        ControlConstants::vescMinERPM,
        ControlConstants::vescMaxERPM
      );

      vesc.setRPM(rpmLimit.calculate(targetValues.erpmCommand));
    } else {
      Serial.println("Bad Value");
    }
  }


  void VescDriver::updateAuto(float erpm, float brake) {
    targetValues.erpmCommand = erpm;
    targetValues.brakeCommand = brake;

    if (brake > 0.0f) {
      vesc.setBrakeCurrent(brake);
      return;
    }

    vesc.setRPM(erpm);
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
    Serial.printf("ERPM - %.2f\t| Brake Current - %.2f\n", targetValues.erpmCommand, targetValues.brakeCommand);
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
