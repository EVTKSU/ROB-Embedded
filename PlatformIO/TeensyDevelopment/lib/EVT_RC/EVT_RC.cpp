// EVT_RC.cpp
#include "EVT_RC.h"

// Keep SBUS dependency isolated to this .cpp so other libs that include EVT_RC.h
// do NOT need to see SBUS.h.
#include <SBUS.h>

// SBUS instance
SBUS sbus(Serial2);

// Storage for externs
uint16_t channels[16] = {0};

uint16_t auto_switch = 0;
uint16_t calibration_switch = 0;
uint16_t reset_switch = 0;

// Timestamp for watchdog (main.cpp reads this)
static volatile uint32_t lastRcFrameMs = 0;

bool sbusFailSafe = false;
bool sbusLostFrame = false;

// Channel mapping (0-based)
// Your main uses channels[5], channels[4], channels[7]. Keep consistent.
static constexpr uint8_t CH_AUTO  = 5;
static constexpr uint8_t CH_CALIB = 4;
static constexpr uint8_t CH_RESET = 7;

void setupSbus() {
  Serial2.begin(100000, SERIAL_8E2);
  sbus.begin();
  delay(200);

  // Prevent immediate false timeout at boot
  lastRcFrameMs = millis();
}

bool updateSbusData() {
  bool gotFrame = sbus.read(channels, &sbusFailSafe, &sbusLostFrame);

  if (gotFrame) {
    // Treat “valid” as “we decoded a frame”; if you want to ignore failsafe frames, gate it:
    if (!sbusFailSafe) {
      lastRcFrameMs = millis();
    }

    auto_switch        = channels[CH_AUTO];
    calibration_switch = channels[CH_CALIB];
    reset_switch       = channels[CH_RESET];
  }

  return gotFrame;
}

uint32_t getLastRcFrameMs() {
  return (uint32_t)lastRcFrameMs;
}
