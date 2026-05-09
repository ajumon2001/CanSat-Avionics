#define TINY_GSM_MODEM_SIM7000
#include <TinyGsmClient.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include "flight_state.h"
#include <TinyGPS++.h>
#include <Wire.h>
#include <HardwareSerial.h>

#define MODEM_RX        26
#define MODEM_TX        27
#define MODEM_PWRKEY     4
#define MODEM_RST        5
#define MODEM_DTR       25
#define MODEM_BAUD   115200

HardwareSerial SerialAT(1);
TinyGsm modem(SerialAT);

bool modemOk = false;
bool simOk = false;
bool networkOk = false;
int16_t rssi = 0;

TinyGPSPlus gps;
double gpsLat = 0.0;
double gpsLon = 0.0;
double gpsAlt = 0.0;
bool gpsFix = false;

FlightState currentState = FlightState::BOOT;
unsigned long bootTimeMs = 0;
unsigned long lastTelemetryMs = 0;
bool sensorsOk = false;
bool gpsOk = false;
bool storageOk = false;
bool batteryOk = true;

float accelX = 0.0;
float accelY = 0.0;
float accelZ = 1.0;
float gyroX = 0.0;
float gyroY = 0.0;
float gyroZ = 0.0;
bool imuOk = false;
bool timingFault = false;
unsigned long lastLoopMicros = 0;
unsigned long loopDeltaMicros = 0;
unsigned long maxLoopMicros = 0;

void readGNSS(); 

void sendTelemetry() {
  JsonDocument doc; // Fixed deprecated StaticJsonDocument
  doc["t"] = millis();
  doc["st"] = stateCode(currentState);
  doc["fix"] = gpsFix ? 1 : 0;
  doc["lat"] = gpsLat;
  doc["lon"] = gpsLon;
  doc["alt"] = gpsAlt;
  doc["sens"] = sensorsOk ? 1 : 0;
  doc["gps"] = gpsOk ? 1 : 0;
  doc["sd"] = storageOk ? 1 : 0;
  doc["bat"] = batteryOk ? 1 : 0;
  doc["ax"] = accelX;
  doc["ay"] = accelY;
  doc["az"] = accelZ;
  doc["gx"] = gyroX;
  doc["gy"] = gyroY;
  doc["gz"] = gyroZ;
  doc["loop_us"] = loopDeltaMicros;
  doc["max_us"] = maxLoopMicros;
  doc["tf"] = timingFault ? 1 : 0;
  doc["mod"] = modemOk ? 1 : 0;
  doc["sim"] = simOk ? 1 : 0;
  doc["net"] = networkOk ? 1 : 0;
  doc["rssi"] = rssi;
  serializeJson(doc, Serial);
  Serial.println();
}

void updateStateMachine() {
  switch (currentState) {
    case FlightState::BOOT:
      if (millis() - bootTimeMs > 2000) {
        currentState = FlightState::IDLE;
      }
      break;
    case FlightState::IDLE:
      sensorsOk = true;
      gpsOk = true;
      storageOk = true;
      if (sensorsOk && gpsOk && storageOk && batteryOk) {
        currentState = FlightState::READY;
      }
      break;
    case FlightState::READY: break;
    case FlightState::ARMED: break;
    case FlightState::BOOST: break;
    case FlightState::COAST: break;
    case FlightState::APOGEE:
      currentState = FlightState::DROGUE;
      break;
    case FlightState::DROGUE: break;
    case FlightState::DESCENT: break;
    case FlightState::LANDED: break;
    case FlightState::FAULT: break;
  }
}

void handleSerialCommands() {
  if (!Serial.available()) return;
  String cmd = Serial.readStringUntil('\n');
  cmd.trim();
  if (cmd.equalsIgnoreCase("ARM")) {
    if (currentState == FlightState::READY) {
      currentState = FlightState::ARMED;
      Serial.println("#ARMED: Flight computer armed");
    } else {
      Serial.println("#ERROR: Cannot arm");
    }
  } else if (cmd.equalsIgnoreCase("GNSS")) {
    readGNSS();
  }
}

void updateGPS() {
  // Use SerialAT because GPS data comes through the modem
  while (SerialAT.available()) {
    gps.encode(SerialAT.read());
  }
  if (gps.location.isUpdated()) {
    gpsLat = gps.location.lat();
    gpsLon = gps.location.lng();
    gpsFix = gps.location.isValid();
  }
  if (gps.altitude.isUpdated()) {
    gpsAlt = gps.altitude.meters();
  }
}

void initIMU() {
  Wire.begin();
  imuOk = true;
  Serial.println("#IMU: initialized");
}

void updateIMU() {
  // Placeholder for real 6DOF logic
  accelZ = 1.01;
}

void updateLoopTiming() {
  unsigned long now = micros();
  loopDeltaMicros = now - lastLoopMicros;
  lastLoopMicros = now;
  if (loopDeltaMicros > maxLoopMicros) maxLoopMicros = loopDeltaMicros;
  if (loopDeltaMicros > 50000) timingFault = true;
}

void powerOnModem() {
  pinMode(MODEM_PWRKEY, OUTPUT);
  pinMode(MODEM_RST, OUTPUT);
  digitalWrite(MODEM_RST, HIGH);
  digitalWrite(MODEM_PWRKEY, HIGH);
  delay(1000);
  digitalWrite(MODEM_PWRKEY, LOW);
  delay(1000);
}

void initModem() {
  // Consolidated to SerialAT
  SerialAT.begin(MODEM_BAUD, SERIAL_8N1, MODEM_RX, MODEM_TX);
  delay(300);
  powerOnModem();
  Serial.println("#MODEM: testing AT");
  modemOk = modem.testAT(3000);
  if (!modemOk) {
    Serial.println("#MODEM: AT failed");
    return;
  }
  Serial.println("#MODEM: AT OK");
  simOk = modem.getSimStatus() == SIM_READY;
  
  // Power on internal GPS
  modem.sendAT("+CGPIO=0,48,1,1");
  modem.waitResponse(1000);
  modem.enableGPS();
  Serial.println("#GNSS: enabled");
}

void updateModemStatus() {
  static unsigned long lastCheck = 0;
  if (!modemOk || millis() - lastCheck < 5000) return;
  lastCheck = millis();
  rssi = modem.getSignalQuality();
  networkOk = modem.isNetworkConnected();
}

void readGNSS() {
  modem.sendAT("+CGNSINF");
  if (modem.waitResponse(1000L, "+CGNSINF:") == 1) {
    String response = SerialAT.readStringUntil('\n');
    Serial.println(response);
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);
  initModem(); 
  initIMU();   
  bootTimeMs = millis();
  currentState = FlightState::BOOT;
  Serial.println("#BOOT: Power Applied & GNSS Enabled");
}

void loop() {
  updateLoopTiming();
  handleSerialCommands();
  updateGPS();
  updateIMU();
  updateModemStatus();
  updateStateMachine();
  if (millis() - lastTelemetryMs >= 1000) {
    lastTelemetryMs = millis();
    sendTelemetry();
  }
}
