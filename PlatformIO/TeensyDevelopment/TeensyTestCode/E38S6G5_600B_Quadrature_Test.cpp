#include <Arduino.h>

// Same drive encoder pin assignment as lib/Constants/IOConstants.hpp.
static constexpr uint8_t encoderPinA = 26;
static constexpr uint8_t encoderPinB = 27;
static constexpr uint16_t encoderPulsesPerRev = 600;
static constexpr double countsPerRotation = encoderPulsesPerRev * 4.0;

static constexpr uint32_t serialBaudrate = 115200;
static constexpr uint32_t printPeriodMs = 100;

static volatile int32_t encoderCount = 0;
static volatile uint32_t invalidTransitions = 0;
static volatile uint8_t lastState = 0;

static uint32_t lastPrintMs = 0;
static int32_t lastPrintedCount = 0;
static uint32_t lastPrintedMs = 0;

static void updateEncoder() {
  const uint8_t newState = (digitalReadFast(encoderPinA) << 1) |
                           digitalReadFast(encoderPinB);
  const uint8_t transition = (lastState << 2) | newState;

  switch (transition) {
    case 0b0001:
    case 0b0111:
    case 0b1110:
    case 0b1000:
      encoderCount++;
      break;

    case 0b0010:
    case 0b1011:
    case 0b1101:
    case 0b0100:
      encoderCount--;
      break;

    case 0b0000:
    case 0b0101:
    case 0b1010:
    case 0b1111:
      break;

    default:
      invalidTransitions++;
      break;
  }

  lastState = newState;
}

static void printPinout() {
  Serial.println();
  Serial.println("E38S6G5-600B quadrature encoder test");
  Serial.println("Teensy-side pins after your 3.3 V level shifter:");
  Serial.printf("  A -> Teensy pin %u\n", encoderPinA);
  Serial.printf("  B -> Teensy pin %u\n", encoderPinB);
  Serial.println("  GND -> Teensy GND and encoder supply GND common");
  Serial.println();
  Serial.println("Common E38 wire colors are often:");
  Serial.println("  Brown/Red: encoder supply +V, Blue/Black: GND, Black: A, White: B");
  Serial.println("Confirm your actual cable before powering it.");
  Serial.println();
  Serial.println("count, turns, delta_count, speed_rps, A, B, invalid_transitions");
}

void setup() {
  Serial.begin(serialBaudrate);
  while (!Serial && millis() < 4000UL) {
  }

  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);

  lastState = (digitalReadFast(encoderPinA) << 1) | digitalReadFast(encoderPinB);

  attachInterrupt(digitalPinToInterrupt(encoderPinA), updateEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoderPinB), updateEncoder, CHANGE);

  lastPrintedMs = millis();
  printPinout();
}

void loop() {
  const uint32_t now = millis();

  if ((now - lastPrintMs) < printPeriodMs) {
    return;
  }

  noInterrupts();
  const int32_t countSnapshot = encoderCount;
  const uint32_t invalidSnapshot = invalidTransitions;
  interrupts();

  const int32_t deltaCount = countSnapshot - lastPrintedCount;
  const uint32_t deltaMs = now - lastPrintedMs;
  const double turns = static_cast<double>(countSnapshot) / countsPerRotation;
  const double speedRps = deltaMs > 0
                            ? (static_cast<double>(deltaCount) / countsPerRotation) / (static_cast<double>(deltaMs) / 1000.0)
                            : 0.0;

  Serial.printf("%ld, %.6f, %ld, %.4f, %u, %u, %lu\n",
                static_cast<long>(countSnapshot),
                turns,
                static_cast<long>(deltaCount),
                speedRps,
                digitalReadFast(encoderPinA),
                digitalReadFast(encoderPinB),
                static_cast<unsigned long>(invalidSnapshot));

  lastPrintedCount = countSnapshot;
  lastPrintedMs = now;
  lastPrintMs = now;
}
