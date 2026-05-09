# LilyGO Primary Flight Computer
## Overview
The `lilygo_primary` firmware acts as the central avionics controller for the CanSat-Avionics system.
It is responsible for:
- flight state management
- telemetry generation
- GPS/GNSS handling
- LTE modem architecture
- future IMU integration
- future SD logging
- system health monitoring
- serial debugging commands
The firmware is intentionally designed using a **non-blocking architecture** so that no subsystem can freeze the flight computer.
---
# Current Features
## Implemented
- Flight state machine
- Serial command interface
- Compact JSON telemetry
- SIM7000G modem initialization
- GNSS/GPS initialization
- Loop timing monitor
- Timing fault detection
## In Progress
- Real GNSS coordinate parsing
- LTE telemetry upload
- ICM-20689 integration
## Planned
- SD blackbox logging
- Sensor fusion
- Watchdog recovery
- FreeRTOS task separation
- Ground dashboard integration
---
# File Structure
```text
lilygo_primary/
├── platformio.ini
├── include/
│   └── flight_state.h
└── src/
    └── main.cpp
```
---
# Main Components
# 1. Flight State Machine
Located in:
```text
include/flight_state.h
```
Defines all valid flight states:
```text
BOOT
IDLE
READY
ARMED
BOOST
COAST
APOGEE
DROGUE
DESCENT
LANDED
FAULT
```
## Why it exists
The state machine ensures deterministic behaviour during flight.
Instead of random sensor-triggered actions, the rocket can only move through predefined flight stages.
This prevents:
- ghost launches
- invalid deployment logic
- unsafe transitions
- accidental state jumps
---
# 2. Telemetry System
Located in:
```text
sendTelemetry()
```
Generates compact JSON telemetry packets.
Example:
```json
{
  "t":12000,
  "st":"ready",
  "lat":55.9221,
  "lon":-3.1724
}
```
## Why compact JSON is used
Compact packets reduce:
- LTE bandwidth usage
- modem transmission time
- memory usage
- telemetry overhead
---
# 3. Serial Command Handler
Located in:
```text
handleSerialCommands()
```
Current commands:
| Command | Function |
|---|---|
| STATUS | Print current flight state |
| ARM | Arm the flight computer |
| GNSS | Print GNSS information |
| SIM | Print SIM status |
---
# 4. SIM7000G Modem System
Located in:
```text
initModem()
updateModemStatus()
```
Responsible for:
- modem initialization
- AT command communication
- GNSS activation
- signal quality monitoring
## Why SIM7000G is used
The SIM7000G combines:
- LTE
- GNSS
- UART communication
into a single compact module.
This reduces:
- wiring complexity
- additional modules
- system weight
---
# 5. GNSS/GPS System
Located in:
```text
updateGPS()
readGNSS()
```
Responsible for:
- satellite lock
- coordinates
- altitude
- future recovery positioning
## Important Note
GPS works independently from LTE.
A SIM card is NOT required for:
- GPS lock
- GNSS coordinates
- satellite tracking
Only the active GPS antenna is required.
---
# 6. IMU System
Located in:
```text
updateIMU()
```
Currently placeholder-safe until hardware integration is completed.
Prepared for:
- ICM-20689
- launch detection
- burnout detection
- orientation estimation
---
# 7. Loop Timing Monitor
Located in:
```text
updateLoopTiming()
```
Measures:
- main loop execution time
- maximum loop latency
- timing instability
## Why this matters
Blocking code can:
- freeze launch detection
- delay deployment logic
- stall telemetry
The timing monitor helps identify unsafe delays early.
---
# Main Loop Structure
```cpp
handleSerialCommands();
updateGPS();
updateIMU();
updateModemStatus();
updateStateMachine();
sendTelemetry();
```
## Why this structure was chosen
This architecture:
- keeps execution deterministic
- prevents subsystem blocking
- simplifies debugging
- improves watchdog safety
- isolates failures
---
# Safety Philosophy
The firmware follows the principle:
## “No single subsystem should crash the flight computer.”
Examples:
- GPS failure should not stop telemetry
- LTE reconnects should not freeze flight logic
- sensor faults should not stop state estimation
---
# Development Notes
## Current Testing Hardware
- LilyGO ESP32 + SIM7000G
- Active GPS antenna
## Future Hardware
- Mikroe ICM-20689 IMU
- XIAO ESP32-S3 Sense
- Eggtimer Quantum
---
# Build Instructions
## Build Firmware
```bash
pio run
```
## Upload Firmware
```bash
pio run -t upload
```
## Open Serial Monitor
```bash
pio device monitor
```
---
# Serial Commands
```text
STATUS
ARM
GNSS
SIM
```
---
# Important Safety Note
This firmware is experimental and currently under active development.

- bypass watchdog protection

---
