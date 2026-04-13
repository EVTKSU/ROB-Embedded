# Changelog

This file documents the current modified lines in `src/main.cpp` and the touched `lib/*` files, with a short comment for each functional change.

Notes:
- This is based on the current working tree diff.
- `TeensyTestCode/FunctionalExamples/ioconstants_sbus.cpp` is not included here because it is a test/example file, not `main` or a library file.
- The large commented legacy block restored inside `src/main.cpp` is grouped as a single note because it is non-runtime reference code.

## `src/main.cpp`

### Added include lines
- `#include "EVT_StateMachine.h"`: Enables active state transitions in the runtime path.
- `#include "EVT_VescDriver.h"`: Allows `main` to call `setupVesc()` and `updateVescControl()`.
- `#include "EVT_Ethernet.hpp"`: Allows `main` to use the current Ethernet class API.
- `#include "EVT_AutoMode.h"`: Keeps the current top-level include set aligned with the state-machine entrypoint.
- `#include "EVT_ODriver.h"`: Allows `main` to call `setupOdrv()` and `updateOdrvControl()`.
- `#include <EVT_RC.hpp>`: Ensures the RC types are available through the new SBUS path.

### Added commented reference lines
- `// bool autonomous = false;`: Restores the old variable as a commented reference only.
- `// int loop_count = 0;`: Restores the old variable as a commented reference only.
- `// int loops_per_telem = 10;`: Restores the old variable as a commented reference only.

### Updated globals
- `unsigned long lastWaitingPrintMs = 0UL;`: Replaces the old print-throttle variable with a wait-message throttle for invalid SBUS frames.

### Setup changes
- `while (!Serial && millis() < 4'000UL) {}`: Waits briefly for USB serial so startup messages are visible.
- `Serial.println("main2 starting");`: Adds startup visibility to the current bring-up path.
- `Serial.println("USB serial uses IOConstants::serialBaudrate");`: Makes the USB serial configuration explicit on boot.
- `Serial.println("SBUS input uses IOConstants::sBusSerial (Serial1 / RX pin 0)");`: Confirms which SBUS UART/pin the runtime is using.
- `digitalWrite(IOConstants::oDriveRelay, HIGH);`: Explicitly powers the ODrive relay during setup.
- `digitalWrite(IOConstants::vescRelay, HIGH);`: Explicitly powers the VESC relay during setup.
- `SetState(NONE);`: Resets the state machine before bring-up.
- `SetState(INIT);`: Starts runtime in initialization mode.
- `Serial.println("Initializing modules...");`: Prints the bring-up stage.
- `setupVesc();`: Initializes the VESC on the current shared serial path.
- `setupOdrv();`: Initializes the ODrive on the current shared serial path.
- `lastUpdate = currentTime;`: Seeds the timed update loop.
- `lastWaitingPrintMs = currentTime;`: Seeds the invalid-frame message throttle.

### Restored commented legacy block
- `/* ... */`: Restores the old state-machine block as commented reference only; it does not run.

### Active loop / SBUS / state-machine changes
- `const bool gotValidFrame = ModuleConstants::transmitter.update();`: Moves SBUS refresh onto the shared `ModuleConstants::transmitter` path every loop.
- `currentTime = millis();`: Updates the loop timestamp after the SBUS read attempt.
- `if (!gotValidFrame) {`: Stops control logic when the RC frame is invalid.
- `if ((currentTime - lastWaitingPrintMs) >= 1'000UL) {`: Rate-limits invalid-frame debug spam.
- `Serial.println("Waiting for valid SBUS frame...");`: Makes loss of valid SBUS visible on serial.
- `lastWaitingPrintMs = currentTime;`: Stores the time of the invalid-frame message.
- `return;`: Prevents stale RC data from driving outputs when SBUS is invalid.
- `if ((currentTime - lastUpdate) >= (ConversionConstants::secToMillis / IOConstants::updateFrequency)) {`: Runs the active control path on the configured update frequency.
- `digitalWrite(IOConstants::ledBuiltIn, HIGH);`: Leaves the status LED on while the active timed loop is running.
- `const uint16_t resetInput = ModuleConstants::transmitter.getChannelValue(Signals::ChannelRC::SWH, false);`: Reads the reset switch from the named RC channel instead of old raw globals.
- `const uint16_t rcInput = ModuleConstants::transmitter.getChannelValue(Signals::ChannelRC::SWF, false);`: Reads the RC-arm switch from the named RC channel.
- `if (GetState() == RC && resetInput > 1500) {`: Adds the runtime `RC -> IDLE` escape on reset-switch press.
- `Serial.println("SWH ▶ IDLE");`: Makes the reset-to-idle transition visible.
- `SetState(IDLE);`: Disables manual RC control by leaving the `RC` branch.
- `if (GetState() == INIT) {`: Keeps ODrive calibration logic isolated to initialization mode.
- `updateOdrvControl();`: Lets ODrive handle SWA-triggered calibration during `INIT`.
- `if (systemInitialized) {`: Checks for successful ODrive initialization completion.
- `SetState(RC);`: Enters manual RC mode immediately after calibration succeeds.
- `} else if (GetState() == IDLE) {`: Adds an explicit idle behavior branch in the active runtime.
- `if (resetInput < 1500 && rcInput > 900) {`: Requires reset released and RC-arm switch active before re-entering RC.
- `Serial.println("SWF ▶ RC");`: Makes the `IDLE -> RC` transition visible.
- `SetState(RC);`: Re-enables manual RC control from `IDLE`.
- `} else if (GetState() == RC) {`: Keeps drivetrain control isolated to RC mode.
- `updateOdrvControl();`: Runs ODrive steering control only while in `RC`.
- `updateVescControl();`: Runs VESC throttle/brake control only while in `RC`.
- `ModuleConstants::ethernet.sendTelemetry();`: Sends telemetry from the active timed loop.
- `lastUpdate = currentTime;`: Stores the last timed-update timestamp.
- `lastWaitingPrintMs = currentTime;`: Resets the invalid-frame message timer after a valid timed cycle.

## `lib/Constants/ModuleConstants.hpp`

- `static Signals::ControlRC transmitter;`: Keeps the shared RC object declaration inside the struct.
- `static Signals::EthernetEVT ethernet;`: Keeps the shared Ethernet object declaration inside the struct.
- Removed `Signals::ControlRC ModuleConstants::transmitter;`: Stops defining the RC singleton in the header.
- Removed `Signals::EthernetEVT ModuleConstants::ethernet;`: Stops defining the Ethernet singleton in the header.
- Removed the old “must be defined outside of the struct” comment block: The real definitions now live in a `.cpp` file instead of the header.

## `lib/Constants/ModuleConstants.cpp`

- `#include "ModuleConstants.hpp"`: Pulls in the shared object declarations.
- `Signals::ControlRC ModuleConstants::transmitter;`: Creates the single shared RC instance in one translation unit.
- `Signals::EthernetEVT ModuleConstants::ethernet;`: Creates the single shared Ethernet instance in one translation unit.

## `lib/EVT_AutoMode/EVT_AutoMode.cpp`

- `#include "EVT_Ethernet.hpp"`: Updates AutoMode to the current Ethernet header name.
- `#include "ModuleConstants.hpp"`: Gives AutoMode access to the shared Ethernet object.
- `using namespace Constants;`: Shortens access to `ModuleConstants`.
- `std::string rawCommands = ModuleConstants::ethernet.receiveUDP();`: Replaces the removed global UDP receive call with the current object API.
- `ModuleConstants::ethernet.sendTelemetry();`: Replaces the removed global telemetry send call with the current object API.

## `lib/EVT_Ethernet/EVT_Ethernet.cpp`

- Removed `ModuleConstants::transmitter.update();`: Keeps SBUS updates owned by `main` / the RC module schedule instead of hiding an extra RC read inside telemetry.

## `lib/EVT_ODriver/EVT_ODriver.h`

- `#include "IOConstants.hpp"`: Gives the header access to the shared LED pin constant.
- `constexpr int STATUS_LED_PIN = Constants::IOConstants::ledBuiltIn;`: Replaces the hard-coded LED macro with the shared IO constant.

## `lib/EVT_ODriver/EVT_ODriver.cpp`

### Shared-config / include changes
- `#include "ModuleConstants.hpp"`: Gives ODrive access to the shared RC object instead of old globals.
- `using namespace Constants;`: Shortens access to shared constants.
- `HardwareSerial &odrive_serial = Constants::IOConstants::oDriveSerial;`: Moves ODrive UART selection onto `IOConstants`.

### Named channel selection
- `constexpr Signals::ChannelRC kSteeringChannel = Signals::ChannelRC::LEFT_X;`: Routes steering through the named RC channel currently chosen for ODrive.
- `constexpr Signals::ChannelRC kCalibrationChannel = Signals::ChannelRC::SWA;`: Routes calibration trigger through the named RC switch.
- `constexpr Signals::ChannelRC kResetChannel = Signals::ChannelRC::SWH;`: Routes reset through the named RC switch.

### Debug / setup messages
- `Serial.println("InitCalibration ▶ SBUS SWA");`: Updates calibration debug text to the named switch.
- `Serial.begin(Constants::IOConstants::serialBaudrate);`: Aligns USB serial setup with `IOConstants`.
- `odrive_serial.begin(Constants::IOConstants::oDriveBaudrate);`: Aligns ODrive UART speed with `IOConstants`.
- `Serial.println("Awaiting SBUS SWA to initCalibration.");`: Updates waiting text to the named switch.

### RC input migration
- `const uint16_t resetInput = ModuleConstants::transmitter.getChannelValue(kResetChannel, false);`: Reads reset from the shared RC object.
- `const uint16_t calibrationInput = ModuleConstants::transmitter.getChannelValue(kCalibrationChannel, false);`: Reads calibration from the shared RC object.
- `const uint16_t steeringInput = ModuleConstants::transmitter.getChannelValue(kSteeringChannel, false);`: Reads steering from the shared RC object.

### Reset handling updates
- `if (resetInput > 1500 && !errorClearFlag) {`: Replaces the old hard-coded `channels[4]` reset check.
- `Serial.println("SWH ▶ clearErrors()");`: Updates debug text to the named reset switch.
- `if (resetInput < 1500) {`: Replaces the old hard-coded reset-release check.

### Calibration trigger updates
- `if (calibrationInput > 900) {`: Replaces the old hard-coded `channels[5]` calibration trigger.
- `Serial.print("Waiting SWA>900 ▶ ");`: Updates waiting text to the named calibration switch.
- `Serial.println(calibrationInput);`: Prints the current named calibration switch value.

### Steering input updates
- `int ch = constrain(steeringInput, 377, 1763);`: Replaces the old hard-coded steering channel read with the shared named RC input.
- `// Steering input → offsetCmd (deadband + mapping), in turns.`: Updates the steering comment to match the current target units.

## `lib/EVT_SlewRateLimiter/EVT_SlewRateLimiter.cpp`

- `-maxDecrease * (timeChange / 1000),`: Fixes the negative clamp bound so the slew limiter can decrease instead of only walking positive.

## `lib/EVT_VescDriver/EVT_VescDriver.cpp`

- `#include "ModuleConstants.hpp"`: Gives VESC code access to the shared RC object.
- `#include "IOConstants.hpp"`: Gives VESC code access to the shared serial selection and baudrate.
- `using namespace Constants;`: Shortens access to shared constants.
- `Constants::IOConstants::vescSerial.begin(Constants::IOConstants::vescBaudrate);`: Moves VESC UART setup onto `IOConstants`.
- `vesc1.setSerialPort(&Constants::IOConstants::vescSerial);`: Uses the shared configured VESC serial port instead of a hard-coded UART.
- `uint16_t *channels = ModuleConstants::transmitter.getValueArray();`: Uses the shared RC data array instead of old RC globals.
