# CanSat-Avionics
Redundant aerospace-style distributed avionics system for high-altitude CanSat / model rocket missions using ESP32-based flight computers, LTE telemetry, GNSS tracking, onboard imaging, and independent recovery systems.
---
# Overview
This project is focused on building a robust, fault-tolerant avionics architecture for:
- Real-time telemetry
- GPS/GNSS tracking
- Flight state estimation
- Sensor fusion
- High-altitude imaging
- Blackbox logging
- Recovery redundancy
- Ground diagnostics
The system is designed using aerospace-style subsystem isolation principles where no single subsystem failure should compromise the entire mission.
---
# System Architecture
| System | Hardware | Mission |
|---|---|---|
| Primary Flight Computer | LilyGO ESP32 + SIM7000G LTE/GNSS | Flight state estimation, telemetry, GPS, logging |
| Imaging Computer | Seeed Studio XIAO ESP32-S3 Sense | Camera/photo/video subsystem |
| Recovery Computer | Eggtimer Quantum | Independent drogue + main deployment |
| Ground Interface | USB-UART + WiFi AP | Bench diagnostics and configuration |
---
# Core Design Philosophy
## Never let one subsystem crash another.
The architecture is intentionally designed to isolate failures.
### Safety-Critical Rules
- Camera task must NEVER block telemetry
- LTE reconnect must NEVER freeze sensor acquisition
- SD card writes must NEVER pause deployment logic
- GPS parsing must NEVER block the flight loop
- WiFi debugging must NEVER run during powered ascent
- Recovery logic must NEVER depend on LTE
---
# Hardware Required
## Primary Flight Computer
- LilyGO ESP32 + SIM7000G
- LTE antenna
- Active GPS antenna
- USB-C cable
## Imaging System
- Seeed Studio XIAO ESP32-S3 Sense
- microSD card
## Sensors
- Mikroe 6DOF IMU Click (ICM-20689)
## Recovery System
- Eggtimer Quantum
## Power
- 2S LiPo
- Buck converter
- Capacitors
- TVS protection
---
# Installation
## Clone Repository
```bash
git clone git@github-personal:ajumon2001/CanSat-Avionics.git
cd CanSat-Avionics
```
---
## Install PlatformIO
```bash
pip install -U platformio
```
---
## Open in VS Code
Install extensions:
- PlatformIO IDE
- C/C++
- GitLens
---
## Build Firmware
```bash
cd firmware/lilygo_primary
pio run
```
---
## Upload Firmware
```bash
pio run -t upload
```
---
## Open Serial Monitor
```bash
pio device monitor
```
---
# GPS Testing
## Connect GPS Antenna
Attach:
- active GPS antenna
- sky-facing orientation recommended
For best results:
- test outdoors
- avoid metal surfaces
- avoid LTE antenna proximity
---
## GNSS Command
Inside serial monitor:
```text
GNSS
```
Expected:
```text
1,1,20260509...,55.xxxx,-3.xxxx,...
```
Meaning:
- GNSS ON
- GPS fix acquired
- coordinates valid
If:
```text
1,0,...
```
GNSS is enabled but no fix acquired yet.
---
# Serial Commands
| Command | Function |
|---|---|
| STATUS | Print current flight state |
| ARM | Arm flight computer |
| GNSS | Print GNSS information |
| SIM | Print SIM status |
Example:
```text
ARM
STATUS
GNSS
```
---
# Flight Software Architecture
## ESP32 Core Allocation
| Core | Responsibility |
|---|---|
| Core 0 | Flight state machine + sensor fusion |
| Core 1 | LTE + GPS + telemetry + logging |
---
# Task Priority Structure
| Priority | Task |
|---|---|
| Highest | Flight state machine |
| High | IMU sampling |
| Medium | GPS parsing |
| Medium | LTE uplink |
| Low | SD logging |
| Lowest | Debug console |
---
# Flight States
```text
#BOOT      : Power Applied
#IDLE      : Sensor calibration + waiting GPS lock
#READY     : All checks green, launch detection enabled
#ARMED     : Final flight-ready mode
#BOOST     : Motor ignition detected
#COAST     : Motor burnout
#APOGEE    : Peak altitude reached
#DROGUE    : Drogue descent phase
#DESCENT   : Main chute deployment phase
#LANDED    : Landing detected + transmit final coordinates
#FAULT     : Fail-safe degraded mode
```
---
# Telemetry Strategy
LTE telemetry is used ONLY for:
- monitoring
- live visualization
- recovery
- post-flight diagnostics
## LTE is NOT deterministic.
At altitude:
- tower handoffs
- network latency
- antenna nulls
- RF interference
can all interrupt connectivity.
Flight-critical decisions are NEVER based on LTE telemetry.
---

# Telemetry Packet
```json
{
  "t":124.52,
  "st":"BOOST",
  "alt":152.4,
  "vel":88.1,
  "acc":14.2,
  "lat":55.9221,
  "lon":-3.1724,
  "bat":3.91,
  "sig":21
}
```
### Telemetry Rate
- 5–10 Hz during ascent
- 1 Hz during descent
---
# Sensor System
## IMU
Mikroe 6DOF IMU Click (ICM-20689)
### Important Considerations
- Launch rail vibration
- Accelerometer clipping
- Gyro saturation
### Recommended Configuration
- Accelerometer: ±16G minimum
- Gyroscope: ±2000 dps
---
# Sensor Filtering
| Sensor | Filtering Method |
|---|---|
| Barometer | Moving average |
| Orientation | Complementary/Kalman filter |
| Launch Detection | Raw acceleration |
---
# Apogee Detection Logic
False apogee triggers can occur due to:
- pressure noise
- Mach effects
- transient acceleration
- vibration spikes
## Multi-condition Apogee Detection
Trigger ONLY if:
```text
Altitude decreasing
AND
Vertical velocity negative
AND
Acceleration near 1G
AND
Minimum boost timeout elapsed
```
---
# Imaging System
## Hardware
Seeed Studio XIAO ESP32-S3 Sense
### Features
- OV2640 camera
- onboard SD card slot
- ESP32-S3R8
- 8MB PSRAM
---
# Imaging Strategy
## Burst Photography
| Flight Phase | Capture Rate |
|---|---|
| Ascent | 5 FPS |
| Near Apogee | 10 FPS |
| Descent | 2 FPS |
Images are stored locally and reconstructed post-flight.
---
# Power Architecture
## Independent Power Rails
| Rail | Devices |
|---|---|
| Recovery Battery | Eggtimer ONLY |
| Avionics Battery | LilyGO + IMU |
| Camera Battery | XIAO ESP32-S3 |
---
# Power System
## Primary Avionics
- 2S LiPo
- Buck converter to 5V
- Local 3.3V regulation
## Protection
- TVS diodes
- LC filtering
- Low-ESR capacitors
### Recommended Capacitors Near LilyGO
- 470uF low ESR
- 100uF tantalum
- 0.1uF ceramic
---
# Thermal Considerations
LTE modules can reach:
- 50–70°C internal temperatures
### Required Mitigations
- airflow paths
- thermal pads
- copper pours
- avoid enclosed foam
---
# GPS Antenna Placement
The GNSS antenna should be:
- sky-facing
- far from LTE antenna
- away from carbon fiber
- away from switching regulators
---
# Recovery Redundancy
## Eggtimer Quantum
Independent:
- battery
- pyro continuity
- deployment logic
Acts as the final recovery safety layer.
---
# Logging Architecture
All telemetry and sensor data is logged locally even if LTE fails.
## Logged Data
- timestamp
- flight state
- IMU data
- GPS coordinates
- battery voltage
- RSSI
- events/faults
---
# Watchdogs
The avionics system uses:
- hardware watchdogs
- task watchdogs
If LTE hangs:
- ONLY communications task should restart
- NEVER the flight state machine
---
# Communication Between Boards
## UART
Used between:
- LilyGO ↔ XIAO ESP32-S3 Sense
Reasons:
- deterministic
- lightweight
- robust
- low overhead
---
# Physical Stack
```text
Top Bay: GPS Antenna
Upper Electronics: LilyGO + LTE
Middle: XIAO Camera
Lower: Eggtimer Recovery
Bottom: Battery + Pyro Isolation
```
---
# Testing Philosophy
A rocket avionics system is judged by:
## how it fails.
Not how it behaves on a desk.
## Mandatory Failure Tests
- LTE disconnect mid-flight
- SD card missing
- GPS loss
- IMU unplugged
- forced watchdog resets
- brownouts
- power rail instability
---
# Repository Structure
```text
CanSat-Avionics/
├── firmware/
│   ├── lilygo_primary/
│   └── xiao_camera/
├── docs/
├── tools/
├── test_data/
└── ground/
```
---
# Current Development Status
## Implemented
- Flight state machine
- Telemetry framework
- Serial command handler
- Loop timing monitor
- SIM7000G modem initialization
- GNSS initialization architecture
- Non-blocking firmware structure
## In Progress
- Real GNSS parsing
- LTE telemetry uplink
- ICM-20689 integration
- SD blackbox logging
## Planned
- Sensor fusion EKF
- Dynamic telemetry rate
- Ground replay dashboard
- Full PCB integration
- Pyro safety logic
- Pre-flight checklist system
---
# Development Environment
## Recommended Tools
- VS Code
- PlatformIO
- GitHub
- ESP32 Arduino Framework
## Main Libraries
- TinyGSM
- TinyGPS++
- ArduinoJson
---
# Development Workflow
Recommended branches:
```text
main
dev
feature/gps
feature/imu
feature/lte
feature/camera
feature/logger
```
Never develop major subsystems directly on `main`.
---
# Safety Notes
## IMPORTANT
This repository contains experimental avionics software.
Never:
- fly untested firmware
- use LTE for flight-critical decisions
- arm without physical safety interlocks
- bypass watchdog logic
- share deployment power rails with avionics rails
Recovery systems should always remain independently powered.
---
# License
This project is currently under active university/student development.
---
# Author
Ajzal Ashraf  
MSc Robotics Engineering  
Heriot-Watt University
GitHub:
https://github.com/ajumon2001
---
