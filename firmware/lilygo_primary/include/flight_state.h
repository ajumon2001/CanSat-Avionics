#pragma once

enum class FlightState {
  BOOT,
  IDLE,
  READY,
  ARMED,
  BOOST,
  COAST,
  APOGEE,
  DROGUE,
  DESCENT,
  LANDED,
  FAULT
};

inline const char* stateCode(FlightState state) {
  switch (state) {
    case FlightState::BOOT:    return "boot";
    case FlightState::IDLE:    return "idle";
    case FlightState::READY:   return "ready";
    case FlightState::ARMED:   return "armed";
    case FlightState::BOOST:   return "boost";
    case FlightState::COAST:   return "coast";
    case FlightState::APOGEE:  return "apogee";
    case FlightState::DROGUE:  return "drogue";
    case FlightState::DESCENT: return "desc";
    case FlightState::LANDED:  return "landed";
    case FlightState::FAULT:   return "fault";
    default:                   return "unk";
  }
}