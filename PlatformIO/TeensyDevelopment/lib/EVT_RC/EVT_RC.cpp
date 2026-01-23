#include "EVT_RC.h"

// SBUS instance
SBUS sbus(Serial2);

// Storage for externs
uint16_t channels[16] = {0};

uint16_t auto_switch = 0;
uint16_t calibration_switch = 0;
uint16_t reset_switch = 0;

bool sbusFailSafe = false;
bool sbusLostFrame = false;

// Channel mapping (0-based)
// Your main uses channels[5], channels[4], channels[7].
// Keep that consistent here.
static constexpr uint8_t CH_AUTO  = 5;
static constexpr uint8_t CH_CALIB = 4;
static constexpr uint8_t CH_RESET = 7;

void setupSbus() {
  // Many SBUS libraries configure the port internally, but Teensy is fine with this explicit setup.
  Serial2.begin(100000, SERIAL_8E2);
  sbus.begin();
  delay(200);
}

bool updateSbusData() {
  // Read a frame into channels[]
  bool gotFrame = sbus.read(channels, &sbusFailSafe, &sbusLostFrame);

  // Update your named switch variables only when we get a fresh frame
  if (gotFrame) {
    auto_switch        = channels[CH_AUTO];
    calibration_switch = channels[CH_CALIB];
    reset_switch       = channels[CH_RESET];
  }

  return gotFrame;
}
