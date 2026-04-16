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
    /**
     * TODO:
     *   - Throw error when reaching the else
    **/
    if (throttleChannel <= TransmitterConstants::deadbandBounds[1] && throttleChannel >= TransmitterConstants::deadbandBounds[0]) {
      targetValues.brakeCommand = 0.0f;
      targetValues.erpmCommand = 0.0f;

      rpmLimit.setLastValue(0.0f);

      vesc.setBrakeCurrent(targetValues.brakeCommand);
      vesc.setCurrent(0.0f);

      Serial.println("In deadband");
    } else if (throttleChannel <= TransmitterConstants::deadbandBounds[0] && targetValues.erpmCommand == 0) {
      targetValues.brakeCommand = map(
        throttleChannel,
        TransmitterConstants::minRC,
        TransmitterConstants::midRC,
        ControlConstants::vescMaxBrake,
        ControlConstants::vescMinBrake
      );

      printState();

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

      printState();

      vesc.setRPM(-(targetValues.erpmCommand));
    } else {
      Serial.println("Bad Value");
    }
  }


  void VescDriver::updateAuto(float erpm, float brake) {
    if (brake > 0.0f) {
      vesc.setBrakeCurrent(brake);
      return;
    }

    vesc.setRPM(-(erpm));
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
    Serial.printf("ERPM - %.3f\t| Brake Current - %.3f\n", targetValues.erpmCommand, targetValues.brakeCommand);
  }


  VescValues VescDriver::getState() {
    return targetValues;
  }
}



// VescUart vesc1;
// String vescDebug = "";
// String vesc1ErrorString;


// float brakeCommand = 0.0;

// bool brakingActive = (brakeCommand > 0.5f); // small threshold to avoid chatter

// void setupVesc() {
//     Serial1.begin(115200);
//     vesc1.setSerialPort(&Serial1);
    
// }

// void updateVescControl() {
//     // Read and clamp the raw SBUS channel value
//     int ch_vesc = constrain(channels[1], 350, 1700);
//     int ch_brake = constrain(channels[2], 330, 1700);

    
//     const int neutral    = 990;
//     const int deadband   = 20;
//     const float maxRPM   = 7500.0f; // old vesc rpm value
//     // const float maxRPM   = 14800.0f // new theoretical vesc value, 3700 Mechancial rpm x 4 pole pairs = 14800 Electrical rpm
//     // btw vesc rpm commands are in electrical rpm not mechanical rpm (this comes from VESC documentation)
//     const int neutral_brake = 1030;
//     const int brake_max = 330;



//     float rpmCommand = 0.0f;

//     // Forward mapping
//     if (ch_vesc > neutral + deadband) {
//         float forwardRange = (1700.0f - (neutral + deadband));
//         rpmCommand = ((ch_vesc - (neutral + deadband)) / forwardRange) * maxRPM;
//     }
//     // Reverse mapping
//     else if (ch_vesc < neutral - deadband) {
//         float reverseRange = ((neutral - deadband) - 350.0f);
//         float revProp = ((neutral - deadband) - ch_vesc) / reverseRange;
//         rpmCommand = -revProp * maxRPM;
//     }
//     // Within deadband → zero
//     else {
//         rpmCommand = 0.0f;
//     }



//     // // brake map

//     if (ch_brake >= neutral_brake - brake_max){
    
//        brakeCommand = 0;
//         Serial.println("no brake!");
//     }
    
//     else{

//     float brakeRange = float(neutral_brake - brake_max);     // e.g., 1030 - 330 = 700
//     float brakeProp  = float(neutral_brake - ch_brake) / brakeRange; // 0..1
//     brakeCommand = brakeProp * 7.0f;          // how many amps we want the brake current to be              
//     vesc1.setBrakeCurrent(brakeCommand);
//        Serial.print("brake current");
//     Serial.print(brakeCommand);
//     }

//     // --- Approach A: Slew‐rate limiting ---
//     static float lastRpm = 0.0f;
//     const float maxDelta = 500.0f;  // max RPM change per loop
//     float delta = rpmCommand - lastRpm;
//     if (delta >  maxDelta) rpmCommand = lastRpm + maxDelta;
//     if (delta < -maxDelta) rpmCommand = lastRpm - maxDelta;
//     lastRpm = rpmCommand;

//     /* 
//     // --- Approach B: Exponential smoothing (alternative) ---
//     // static float lastRpm = 0.0f;
//     // const float alpha = 0.2f;  // between 0 (smooth) and 1 (responsive)
//     // rpmCommand = alpha * rpmCommand + (1 - alpha) * lastRpm;
//     // lastRpm    = rpmCommand;
//     */

//  // --- NEW: Coast in neutral, speed mode otherwise ---
// bool inDeadband = (ch_vesc >= neutral - deadband) && (ch_vesc <= neutral + deadband);



//     // --- NEW: Braking gate (prevents RPM commands this loop if braking is active)
// bool brakingActive = (brakeCommand > 0.5f); // small threshold avoids chatter around zero
// if (brakingActive) {
//     // Brake was already sent above; just avoid sending RPM in the same loop
//     return;
// }
// // brake didnt work because when we called the brake it kept running the loop and set rpm right after calling the brake.
// //fixed! 


// if (inDeadband) {
//     // Switch to current mode with 0 A to avoid auto-braking in speed mode
    
//     vesc1.setBrakeCurrent(0.0f);
//     vesc1.setCurrent(0.0f);     // If your wrapper lacks setCurrent(), use setDuty(0.0f) instead.
// } else {
    
//     vesc1.setRPM(rpmCommand);
//     Serial.print("brake current");
//     Serial.print(brakeCommand);
//     Serial.println();

// }
// }

// void updateVescControl(float throttle_percent) {

//     // mappint -100 -> +100 to 350 -> 1700
//     int mapped_throttle = int(throttle_percent * 75);
//     Serial.println();
//     Serial.print("mapped throttle: ");
//     Serial.print(mapped_throttle);
//     Serial.print("brake current");
//     Serial.print(brakeCommand);
//     Serial.println();

//     // Send the RPM command to the VESC
//     vesc1.setRPM(mapped_throttle);

    
    
// }


