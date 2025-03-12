#define TINY_GSM_MODEM_SIM7080  // Define the GSM modem model
#define MODEM_SERIAL Serial1    // Define the serial port for the modem

#include "settings_functions.h"
#include "divices/GPS.h"
#include "divices/LTE.h"
#include <StreamDebugger.h>  // Include StreamDebugger library

// Create modem instance first
TinyGsm modem(MODEM_SERIAL);  // Define modem instance
TinyGsmClient client(modem);  // Instantiate client with the modem

GPS gps;
Settings settings;
LTE lte;  // Declare LTE instance outside of the function

bool running = false;
bool GPSON = true;
bool GPRSon = true;
int GPSDaleyGetCords = 1000;

// StreamDebugger setup
StreamDebugger debugger(MODEM_SERIAL, Serial);  // Initialize StreamDebugger with modem serial and Serial output

void setup() {
  Serial.begin(115200);
  delay(3000);
  Serial.println("115200 serial started");

  // Load settings from file or initialize defaults
  if (!settings.loadSettings()) {
    settings.initializeDefaults();
    settings.saveSettings();
  }
}

bool isModemInitialized() {
  return modem.testAT(1000);  // Returns true if modem responds to AT commands
}

bool isGPSEnabled() {
  modem.sendAT("AT+CGNSINF");
  int8_t responseStatus = modem.waitResponse();  // Wait for a response

  if (responseStatus == 0) {
    return true;  // GPS is enabled
  } else {
    Serial.println("GPS: No response from modem, GPS check failed.");
  }

  return false;  // GPS is not enabled
}

void loop() {
  if (Serial.available() > 0) {
    String command = Serial.readString();
    if (command == "stop") {
      running = false;
      Serial.println("Loop stopped.");
    } else if (command == "start") {
      running = true;
      Serial.println("Loop started.");
    }
  }

  if (running) {
    if (GPSON) {
      Serial.println("GPS true");
      GPSOnGetData();
    }

    if (GPRSon) {
      GPRSGetData();
    }
  }

  // Handle debugger messages
  if (debugger.available()) {
    String debugMsg = debugger.readString();
    Serial.println("Debug: " + debugMsg);
  }
}

void GPRSGetData() {
  if (!lte.isConnected()) {
    Serial.println("GPRS: Modem is not connected. Initializing...");

    // Attempt to connect to GPRS
    lte.GPRSGetData();  // Call GPRSGetData() to connect to the network

    // Check connection status
    if (lte.isConnected()) {
      Serial.println("GPRS: Connected to GPRS.");
    } else {
      Serial.println("GPRS: Failed to connect to GPRS.");
    }
  } else {
    Serial.println("GPRS: Already connected to GPRS.");
  }
}

void GPSOnGetData() {
  Serial.println("GPS trueeeeeeeee");
  if (!isModemInitialized()) {
    Serial.println("GPS: Modem is not initialized. Reinitializing...");
    gps.initializePowerChip();
    delay(2000);
  }

  if (!modem.testAT(1000)) {
    Serial.println("GPS: Modem not activated. Reinitializing...");
    gps.activateModem();
    delay(2000);
  }

  if (!isGPSEnabled()) {
    Serial.println("GPS: GPS not enabled. Enabling...");
    modem.enableGPS();
    delay(20000);
  }

  float lat, lon, speed, alt, accuracy, course;
  int vsat, usat, year, month, day, hour, min, sec;

  if (gps.getCoordinates(&lat, &lon, &speed, &alt, &vsat, &usat, &accuracy, &year, &month, &day, &hour, &min, &sec, &course)) {
    Serial.print("Lat: ");
    Serial.println(lat, 6);
    Serial.print("Lon: ");
    Serial.println(lon, 6);
    Serial.print("Speed: ");
    Serial.println(speed);
    Serial.print("Altitude: ");
    Serial.println(alt);
    Serial.print("Course: ");
    Serial.println(course);
    Serial.print("VSAT: ");
    Serial.println(vsat);
    Serial.print("USAT: ");
    Serial.println(usat);
    Serial.print("Accuracy: ");
    Serial.println(accuracy);
    Serial.print("Date: ");
    Serial.print(year);
    Serial.print("-");
    Serial.print(month);
    Serial.print("-");
    Serial.println(day);
    Serial.print("Time: ");
    Serial.print(hour);
    Serial.print(":");
    Serial.print(min);
    Serial.print(":");
    Serial.println(sec);
    delay(GPSDaleyGetCords);
  } else {
    Serial.println("Failed to get GPS data");
  }
}
