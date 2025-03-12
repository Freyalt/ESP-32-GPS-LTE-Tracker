#ifndef GPS_H
#define GPS_H

#include <Arduino.h>
#define XPOWERS_CHIP_AXP2101
#include "XPowersLib.h"
#include "utilities/utilities.h" // Correct path to utilities.h
#include <TinyGsmClient.h>

// Declare modem as extern to avoid redefinition
extern TinyGsm modem; // Ensure this is defined in one of your source files

XPowersPMU PMU;

#define TINY_GSM_RX_BUFFER 1024
#define TINY_GSM_MODEM_SIM7080

class GPS {
public:
    // Function to initialize the power chip and enable modem and GPS power
    bool initializePowerChip() {
        Serial.println("GPS: Starting power chip initialization...");
        if (!PMU.begin(Wire, AXP2101_SLAVE_ADDRESS, I2C_SDA, I2C_SCL)) {
            Serial.println("GPS: Failed to initialize power.....");
            return false; // Indicate failure
        }

        // Set the voltages for the modem and GPS
        PMU.setDC3Voltage(3000);    // Set modem voltage to 3000 mV
        PMU.enableDC3();            // Enable DC3 channel for the modem

        PMU.setBLDO2Voltage(3300);  // Set GPS antenna voltage to 3300 mV
        PMU.enableBLDO2();          // Enable BLDO2 channel for GPS

        // Disable TS Pin detection
        PMU.disableTSPinMeasure();
        
        return true; // Indicate success
    }

    void activateModem() {
        // Modem activation code...
        Serial1.begin(115200, SERIAL_8N1, BOARD_MODEM_RXD_PIN, BOARD_MODEM_TXD_PIN);
        pinMode(BOARD_MODEM_PWR_PIN, OUTPUT);
        
        digitalWrite(BOARD_MODEM_PWR_PIN, LOW);
        delay(100);
        digitalWrite(BOARD_MODEM_PWR_PIN, HIGH);
        delay(1000);
        digitalWrite(BOARD_MODEM_PWR_PIN, LOW);

        int retry = 0;
        while (!modem.testAT(1000)) {
            Serial.print("GPS: ");
            if (retry++ > 15) {
                // Restart modem if needed
                digitalWrite(BOARD_MODEM_PWR_PIN, LOW);
                delay(100);
                digitalWrite(BOARD_MODEM_PWR_PIN, HIGH);
                delay(1000);
                digitalWrite(BOARD_MODEM_PWR_PIN, LOW);
                retry = 0;
                Serial.println("Retry start modem.");
            }
        }
        Serial.println("GPS: Modem activated.");

        modem.enableGPS(); 
        Serial.println("GPS Enabled.");
    }

    // Function to get GPS coordinates
    bool getCoordinates(float *lat, float *lon, float *speed, float *alt, int *vsat, int *usat, float *accuracy, int *year, int *month, int *day, int *hour, int *min, int *sec, float *course) {
        if (modem.getGPS(lat, lon, speed, alt, vsat, usat, accuracy, year, month, day, hour, min, sec)) {
            // Course might be included in the GPS data from the modem
            *course = *speed; // Placeholder: Modify this according to how your modem provides course data
            return true; // Successfully retrieved GPS data
        }
        return false; // Failed to retrieve GPS data
    }

    // Function to deactivate the modem
    void deactivateModem() {
        Serial.println("GPS: Deactivating modem...");
        Serial1.end();  // Stop serial communication
        delay(1000);  // Allow time for the modem to shut down
        Serial.println("GPS: Modem deactivated.");
    }
};

#endif
