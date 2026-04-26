# ROB‑Embedded Teensy Firmware

**Table of Contents**

1. [PlatformIO Setup IMPORTANT](#platformio-setup-important)  
2. [Build and Flash Workflow](#build-and-flash-workflow)  
3. [Module Overview](#module-overview)  
   * [State Machine EVT_StateMachine](#state-machine-evt_statemachine)  
   * [Ethernet and Telemetry EVT_Ethernet](#ethernet-and-telemetry-evt_ethernet)  
   * [RC Interface EVT_RC](#rc-interface-evt_rc)  
   * [VESC Driver EVT_VescDriver](#vesc-driver-evt_vescdriver)  
   * [Odrive Driver EVT_ODriver](#odrive-driver-evt_odriver)  
   * [Autonomous Mode EVT_AutoMode](#autonomous-mode-evt_automode)  
4. [Runtime Flow](#runtime-flow)  
5. [Extending the Code Base](#extending-the-code-base)  
6. [Troubleshooting FAQ](#troubleshooting-faq)  

---

## PlatformIO Setup IMPORTANT

### How to run code
  - Open the TeensyDevelopment folder in VSCode **(BY ITSELF)**
  - Wait for platformio to load and configure
  - Open src\main.cpp
  - Check mark icon compiles code
  - -> arrow icon uploads code
  - Plug icon displays serial monitor

The **TeensyDevelopment** folder is a stand‑alone PlatformIO project.

* **Only one file may live in `src/` at a time** – that file must be `main.cpp`.  
* All reusable code lives in `lib/` as named library folders (e.g. `lib/EVT_RC/…`).  

* **Storing prototypes / experiments**

  * Drop extra sketches in `TeensyTestCode/`.  
  * Comment your name at the top, then copy‑paste the file into `src/main.cpp` when you actually want to run it.

---

## Build and Flash Workflow

1. **Connect** the Teensy and open VS Code.  
2. **Configure** any IP/MAC changes in `EVT_Ethernet.cpp` (default `192.168.0.177`).  
3. **Build** (✔️) – PlatformIO compiles every library under `lib/`.  
4. **Upload** (➡️) – Flashes the Teensy; the board will reboot.  
5. **Monitor** (🔌) – Opens serial @ 9600 baud; watch debug prints.  

> **Note:** Ethernet MAX PACKET SIZE in the Teensy core must be raised to 64 bytes (default is 36).  
> Edit `<Arduino‑core>/libraries/NativeEthernet/src/utility/util.h` if you have compile‑time truncation issues.

---

## Module Overview

### State Machine EVT_StateMachine

| Enum State | Purpose |
|------------|---------|
| `NONE`     | Pre‑boot / undefined |
| `IDLE`     | Ready / waiting for operator |
| `RC`       | Manual remote‑control mode |
| `AUTO`     | Autonomous mode running UDP commands |
| `ERROR`    | Fatal error – motors stopped, requires user reset |
| `STOP`     | Reserved stop state |
| `RESET`    | ODrive reset / recovery state |

Modules call `setState()` or `setErrorState()` to transition. `toString()` converts the enum to a printable string.

---

### Ethernet and Telemetry EVT_Ethernet

* Initializes **NativeEthernet** and a global `EthernetUDP Udp` object.  
* `sendTelemetry()` — sends CSV telemetry to panda host on UDP port `8888`.  
* `receiveUDP()` — non‑blocking; returns a `std::string` packet or empty.

---

### RC Interface EVT_RC

* Uses **SBUS** on `Serial2` @ 100 kBd.  
* Exposes `uint16_t channels[10]` array.  
* `updateSbusData()` refreshes the channel buffer – called every loop.

---

### VESC Driver EVT_VescDriver

* Two **VescUart** objects (`Serial1`, `Serial5`).  
* Maps `channels[1]` (throttle) to ±7500 RPM with neutral dead‑band.  
* Updates global `vescDebug` string with live RPM & voltage.

---

### Odrive Driver EVT_ODriver

* UART on **Serial6**.  
* Handles motor & encoder offset calibration (triggered via `channels[5]`).  
* Supports error clearing / re‑cal via `channels[4]`.  
* Controls steering position via `channels[3]`.  
* Publishes `odrvDebug` for telemetry prints.

---

### Autonomous Mode EVT_AutoMode

* Main runtime path is `ModuleConstants::autoDriver.updateAuto(ModuleConstants::ethernet.receiveUDP())`.
* Expected UDP packet is exactly `erpm,steering_degrees,emergency,state`.
* Remote `state` values `ESTOP` / `E-STOP` / `EMERGENCY_STOP` force error stop.
* Remote `state` values `HOLD` / `MANUAL` / `IDLE` / `RC` / `STOP` command neutral steering + neutral drive.

---

## Runtime Flow

1. **setup()**  
   * Initializes ODrive + Ethernet bringup.  
   * `setState(IDLE)` – ready for manual driving.

2. **loop()**  
   * Always refresh SBUS.  
   * Runs current state handler  
     * **RC** – if `channels[6] > 1000` ➜ `AUTO`, else run VESC & ODrive updates.  
     * **AUTO** – if `channels[6] < 1000` ➜ back to `RC`; otherwise run UDP autonomous routine.  
     * **ERROR** – wait for operator reset via `SWH`.  

3. **Telemetry** – autonomous loop always sends telemetry; RC loop can be extended later.

---

## Extending the Code Base

* **New module?** Create `lib/EVT_MyModule/` with `EVT_MyModule.h` / `EVT_MyModule.cpp`.  
* **Error handling** – call `setErrorState()`.  
* **Documentation** – each library needs a `README.md` explaining its API.  
* **Branches** – develop on a new Git branch; open PRs for review.

---

## Troubleshooting FAQ

| Problem | Fix |
|---------|-----|
| **“multiple definition of operator new”** | Add `-Wl,--allow-multiple-definition` to `build_flags` in `platformio.ini`. |
| **Ethernet packet cut at 36 B** | Increase `UDP_TX_PACKET_MAX_SIZE` to 64 in Teensy **NativeEthernet** core. |
| **State machine keeps breaking** | Follow enum + `switch` template in `main.cpp`; keep module code non‑blocking. |
| **No SBUS data** | Confirm `Serial2` wiring and 100 kBd 8E2 settings. |
| **ODrive never reaches CLOSED_LOOP** | Check power, hall/encoder cables, and run calibration trigger (`channels[5]`). |

---

> *“Making a proper state machine setup nukes the code every time – until this one.”*
 
## Computing Systems and Task Distribution Diagram
![software_diagram](README.assets/software_diagram.png)
