// LTE.h
#ifndef LTE_H
#define LTE_H

#include <TinyGsmClient.h>
#include "utilities/utilities.h"

extern TinyGsm modem; // Declare modem as extern to avoid redefinition

// GPRS credentials
const char* apn = "omnitel";  // Your APN
const char* user = "";         // Your username
const char* pass = "";         // Your password

class LTE {
public:
    LTE() {}

    void begin() {
        modem.restart();
        modem.gprsConnect(apn, user, pass);
        Serial.println("LTE initialized.");
    }

    bool isConnected() {
        return modem.isGprsConnected();
    }

    void GPRSGetData() {
        if (!isConnected()) {
            Serial.println("Connecting to GPRS...");
            if (modem.gprsConnect(apn, user, pass)) {
                Serial.println("Connected to GPRS.");
            } else {
                Serial.println("Failed to connect to GPRS.");
            }
        } else {
            Serial.println("Already connected to GPRS.");
        }
    }

    void disconnect() {
        modem.gprsDisconnect();
        Serial.println("Disconnected from GPRS.");
    }
};

#endif
