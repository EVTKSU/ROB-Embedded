#include <SPI.h>
#include <sstream>
#include <cstring>
#include <cstdlib>                // strtod/strtol
#include "EVT_VescDriver.h"
#include "EVT_ODriver.h"
#include "EVT_Ethernet.h"
#include "EVT_STATEMACHINE.h"     // SetErrorState, STATE, etc.

// ----------------------
// Globals (control inputs)
// ----------------------
float erpm = 0.0f;

float throttle = 0.0f;            // 0..100 expected (negative allowed but ignored in CtrlVesc)
float steering = 0.0f;            // -100..100 expected
bool  emergency = false;
bool  urgentStop = false;         // not in packet yet (still local)

// New packet fields
float brakePct = 0.0f;            // 0..100 from packet
float auxPct = 0.0f;              // unused placeholder
STATE packetState = AUTO;         // requested state from packet
bool  packetOk = true;            // last packet validity flag

// ----------------------
// Derived command values (telemetry/debug)
// ----------------------
float brakeCurrent = 0.0f;        // A (derived from brakePct)
float throttleRpm = 0.0f;         // electrical rpm command
float SteeringPos = 0.0f;         // odrive turns command

enum class BrakeState { Off, Ebrake, Dynamic };
BrakeState brakeState = BrakeState::Off;
bool coasting = false;

static constexpr float MAX_BRAKE_A = 20.0f;
static constexpr float MAX_RPM_ELEC = 7500.0f;     // electrical rpm
static constexpr float MAX_STEER_TURNS = 2.25f;    // odrive turns at 100%
static constexpr float STEER_DEADBAND = 0.25f;     // percent deadband

// If you want steering disabled in ERR/INIT/etc, set this true.
// If false, steering always follows packet even in ERR.
static constexpr bool DISABLE_STEER_ON_ERR = false;

// ----------------------
// Helpers
// ----------------------
static bool parseFloatStrict(const char* s, float& out) {
    char* end = nullptr;
    double v = strtod(s, &end);
    if (end == s) return false;
    out = (float)v;
    return true;
}

static bool parseIntStrict(const char* s, int& out) {
    char* end = nullptr;
    long v = strtol(s, &end, 10);
    if (end == s) return false;
    out = (int)v;
    return true;
}

static bool intToState(int raw, STATE& out) {
    if (raw < (int)NONE || raw > (int)ERR) return false;
    out = (STATE)raw;
    return true;
}

static const char* brakeStateName(BrakeState s) {
    switch (s) {
        case BrakeState::Off:     return "OFF";
        case BrakeState::Ebrake:  return "EBRAKE";
        case BrakeState::Dynamic: return "DYNAMIC";
        default:                  return "UNKNOWN";
    }
}

// ----------------------
// Packet parsing
// Expected format: "throttle,steering,emergency,state,brake,aux"
// ----------------------
void setControls(const std::string &udpData) {
    packetOk = true;

    char udpCopy[128];
    strncpy(udpCopy, udpData.c_str(), sizeof(udpCopy) - 1);
    udpCopy[sizeof(udpCopy) - 1] = '\0';

    char* token = strtok(udpCopy, ",");
    int index = 0;

    // Temporaries: commit only if valid
    float t_throttle = throttle;
    float t_steering = steering;
    bool  t_emergency = emergency;
    STATE t_state = packetState;
    float t_brakePct = brakePct;
    float t_auxPct = auxPct;

    while (token != nullptr) {
        switch (index) {
            case 0: {
                float v;
                if (!parseFloatStrict(token, v)) { packetOk = false; break; }
                t_throttle = v;  // allow negative; CtrlVesc handles it (safe return)
            } break;

            case 1: {
                float v;
                if (!parseFloatStrict(token, v)) { packetOk = false; break; }
                t_steering = v;
            } break;

            case 2: {
                int v;
                if (!parseIntStrict(token, v)) { packetOk = false; break; }
                t_emergency = (v != 0);
            } break;

            case 3: {
                int v;
                if (!parseIntStrict(token, v) || !intToState(v, t_state)) {
                    packetOk = false;
                    break;
                }
            } break;

            case 4: {
                float v;
                if (!parseFloatStrict(token, v)) { packetOk = false; break; }
                if (v < 0.0f || v > 100.0f) packetOk = false;  // mark bad, but clamp anyway
                if (v < 0.0f) v = 0.0f;
                if (v > 100.0f) v = 100.0f;
                t_brakePct = v;
            } break;

            case 5: {
                float v;
                if (!parseFloatStrict(token, v)) { packetOk = false; break; }
                t_auxPct = v; // unused placeholder
            } break;

            default:
                // Extra fields ignored (keep packetOk as-is)
                break;
        }

        index++;
        token = strtok(nullptr, ",");
    }

    if (index < 6) packetOk = false;

    if (!packetOk) {
        // Fail-safe: error state + force EBRAKE via emergency flag
        SetErrorState(ERR_ETHERNET, "Bad control packet");
        emergency = true;
        packetState = ERR;
        return;
    }

    // Commit validated values
    throttle    = t_throttle;
    steering    = t_steering;
    emergency   = t_emergency;
    packetState = t_state;
    brakePct    = t_brakePct;
    auxPct      = t_auxPct;
}

// ----------------------
// VESC control
// - brakePct (0..100) -> 0..20A
// - throttle negative: safe return (no error yet)
// ----------------------
void CtrlVesc() {
    // If you do NOT want packet to drive state machine, remove this block.
    if (packetState != CurrentState) {
        SetState(packetState);
    }

    // Priority 1: emergency / urgentStop -> EBRAKE
    if (emergency || urgentStop) {
        brakeState   = BrakeState::Ebrake;
        brakeCurrent = MAX_BRAKE_A;
        coasting     = false;
        throttleRpm  = 0.0f;

        vesc1.setCurrent(0.0f);
        vesc1.setBrakeCurrent(brakeCurrent);
        return;
    }

    // If throttle is negative, ignore for now (no error). Safe-coast.
    if (throttle < 0.0f) {
        brakeState   = BrakeState::Off;
        brakeCurrent = 0.0f;
        coasting     = true;
        throttleRpm  = 0.0f;

        vesc1.setBrakeCurrent(0.0f);
        vesc1.setCurrent(0.0f);
        return;
    }

    // Brake from packet (0..100 -> 0..20A). If braking, do NOT send RPM same loop.
    if (brakePct > 0.5f) {
        brakeState   = BrakeState::Dynamic;
        brakeCurrent = (brakePct / 100.0f) * MAX_BRAKE_A;
        coasting     = false;
        throttleRpm  = 0.0f;

        vesc1.setCurrent(0.0f);
        vesc1.setBrakeCurrent(brakeCurrent);
        return;
    }

    // Neutral throttle -> coast
    if (throttle == 0.0f) {
        brakeState   = BrakeState::Off;
        brakeCurrent = 0.0f;
        coasting     = true;
        throttleRpm  = 0.0f;

        vesc1.setBrakeCurrent(0.0f);
        vesc1.setCurrent(0.0f);
        return;
    }

    // Positive throttle -> RPM (explicitly clear brake current)
    brakeState   = BrakeState::Off;
    brakeCurrent = 0.0f;
    coasting     = false;

    float t = throttle;
    if (t > 100.0f) t = 100.0f;

    throttleRpm = (t / 100.0f) * MAX_RPM_ELEC;

    vesc1.setBrakeCurrent(0.0f);
    vesc1.setRPM(throttleRpm);
}

// ----------------------
// ODrive steering control (RESTORED)
// ----------------------
void CtrlOdrive() {
    if (DISABLE_STEER_ON_ERR && (CurrentState == ERR)) {
        SteeringPos = 0.0f;
        odrive.trapezoidalMove(SteeringPos);
        return;
    }

    // Optional clamp
    float s = steering;
    if (s > 100.0f)  s = 100.0f;
    if (s < -100.0f) s = -100.0f;

    // Deadband and mapping
    if (s < -STEER_DEADBAND) {
        SteeringPos = (s / 100.0f) * MAX_STEER_TURNS;
    } else if (s > STEER_DEADBAND) {
        SteeringPos = (s / 100.0f) * MAX_STEER_TURNS;
    } else {
        SteeringPos = 0.0f;
    }

    odrive.trapezoidalMove(SteeringPos);
}

// ----------------------
// Autonomous update loop
// ----------------------
void updateAutonomousMode() {
    odrvDebug = "Autonomous mode active.";

    // Read UDP first so controls take effect this cycle
    std::string rawCommands = receiveUdp();
    if (!rawCommands.empty()) {
        setControls(rawCommands);
    }

    CtrlVesc();
    CtrlOdrive();
   // sendAutoTelemetry();

    // Debug print (post-apply)
    Serial.print(" | Throttle(%): ");
    Serial.print(throttle);
    Serial.print(" Throttle(rpm): ");
    Serial.print(throttleRpm);

    Serial.print(" | Steering(%): ");
    Serial.print(steering);
    Serial.print(" Steering(turns): ");
    Serial.print(SteeringPos);

    Serial.print(" | Emergency: ");
    Serial.print(emergency ? "YES" : "NO");

    Serial.print(" | Brake(%): ");
    Serial.print(brakePct);
    Serial.print(" Brake(A): ");
    Serial.print(brakeCurrent);

    Serial.print(" | BrakeState: ");
    Serial.print(brakeStateName(brakeState));

    Serial.print(" | Coasting: ");
    Serial.print(coasting ? "YES" : "NO");

    Serial.print(" | PacketState: ");
    Serial.print(StateToString(packetState));

    Serial.print(" | CurrentState: ");
    Serial.println(StateToString(CurrentState));
}
