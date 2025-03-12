#ifndef GPS_H
#define GPS_H

#include <Arduino.h>
#include <HardwareSerial.h> // Naudojame serijinę sąsają
#include "utilities/utilities.h" // Include utilities for pin definitions

class GPS {
  private:
    bool gpsPowerStatus = false;  // GPS imtuvo statusas (true - įjungtas, false - išjungtas)
    HardwareSerial* gpsSerial;    // Serijinė sąsaja GPS ryšiui
    float latitude = 0.0;         // Dabartinė platuma
    float longitude = 0.0;        // Dabartinė ilguma
    String date;                  // Nustatyta data
    String time;                  // Nustatytas laikas
    unsigned long startMillis;    // Laikas, kai buvo gauti GPS duomenys

    // Funkcija siunčia AT komandas į SIM7080 modulį ir laukia atsakymo
    String sendATCommand(String command, int timeout = 1000) {
      gpsSerial->println(command);
      String response = "";
      long int time = millis();
      while ((time + timeout) > millis()) {
        while (gpsSerial->available()) {
          char c = gpsSerial->read();
          response += c;
        }
      }
      return response;
    }

    // Helper function to pad a number with a leading zero if needed
    String padZero(int number) {
      return (number < 10) ? "0" + String(number) : String(number);
    }

  public:
    // Constructor now accepts only the HardwareSerial pointer
    GPS(HardwareSerial* serial) {
      gpsSerial = serial;
    }

    // Funkcija inicializuoja serijinę sąsają
    void begin(int baudRate = 9600) {
      gpsSerial->begin(baudRate, SERIAL_8N1, BOARD_MODEM_RXD_PIN, BOARD_MODEM_TXD_PIN);
      Serial.println("GPS serijinė sąsaja inicijuota");
    }

    // Funkcija patikrinti GPS imtuvo statusą
    bool isGPSEnabled() {
      return gpsPowerStatus;
    }

    // Funkcija įjungti GPS imtuvą
    void enableGPS() {
      gpsPowerStatus = true;
      Serial.println("GPS imtuvas įjungtas");
      sendATCommand("AT+CGNSPWR=1"); // Įjungiam GPS maitinimą per AT komandą
    }

    // Funkcija išjungti GPS imtuvą
    void disableGPS() {
      gpsPowerStatus = false;
      Serial.println("GPS imtuvas išjungtas");
      sendATCommand("AT+CGNSPWR=0"); // Išjungiam GPS maitinimą per AT komandą
    }

    // Funkcija gauti dabartines koordinates ir laiką iš SIM7080 moduliu per AT komandas
    bool getCoordinates(float &lat, float &lng, String &dateStr, String &timeStr) {
      if (gpsPowerStatus) {
        String response = sendATCommand("AT+CGNSINF", 2000);
        Serial.println("AT+CGNSINF Response: " + response);

        if (response.indexOf("+CGNSINF: 1,") != -1) {
          int latStart = response.indexOf(",", response.indexOf("+CGNSINF: 1,") + 12) + 1;
          int latEnd = response.indexOf(",", latStart);
          int lngStart = response.indexOf(",", latEnd + 1) + 1;
          int lngEnd = response.indexOf(",", lngStart);
          int utcStart = response.indexOf(",", latEnd) + 1;
          String utcString = response.substring(utcStart, response.indexOf(",", utcStart));

          int yearUTC = utcString.substring(0, 2).toInt() + 2000;
          int monthUTC = utcString.substring(3, 5).toInt();
          int dayUTC = utcString.substring(6, 8).toInt();
          int hourUTC = utcString.substring(9, 11).toInt();
          int minuteUTC = utcString.substring(12, 14).toInt();
          int secondUTC = utcString.substring(15, 17).toInt();

          lat = response.substring(latStart, latEnd).toFloat();
          lng = response.substring(lngStart, lngEnd).toFloat();

          dateStr = String(yearUTC) + "-" + padZero(monthUTC) + "-" + padZero(dayUTC);
          timeStr = padZero(hourUTC) + ":" + padZero(minuteUTC) + ":" + padZero(secondUTC);

          startMillis = millis();

          Serial.print("Koordinatės: Platuma = ");
          Serial.print(lat, 6);
          Serial.print(", Ilguma = ");
          Serial.println(lng, 6);
          Serial.print("Data: ");
          Serial.println(dateStr);
          Serial.print("Laikas: ");
          Serial.println(timeStr);

          return true;
        } else {
          Serial.println("Negalima gauti palydovų informacijos.");
          getSatellitesInfo();
          return false;
        }
      } else {
        Serial.println("GPS imtuvas išjungtas, negalima gauti koordinačių.");
        return false;
      }
    }

    // Funkcija gauti palydovų informaciją
    void getSatellitesInfo() {
      String statusResponse = sendATCommand("AT+CGNSSTATUS", 2000);
      Serial.println("AT+CGNSSTATUS Response: " + statusResponse);

      int runStatusIndex = statusResponse.indexOf("Run Status");
      int fixStatusIndex = statusResponse.indexOf("Fix Status");
      int satellitesUsedIndex = statusResponse.indexOf("Satellites Used");
      int signalQualityIndex = statusResponse.indexOf("Signal Quality");

      if (runStatusIndex != -1 && fixStatusIndex != -1 &&
          satellitesUsedIndex != -1 && signalQualityIndex != -1) {
        Serial.print("Run Status: ");
        Serial.println(statusResponse.substring(runStatusIndex, statusResponse.indexOf(",", runStatusIndex)));
        Serial.print("Fix Status: ");
        Serial.println(statusResponse.substring(fixStatusIndex, statusResponse.indexOf(",", fixStatusIndex)));
        Serial.print("Satellites Used: ");
        Serial.println(statusResponse.substring(satellitesUsedIndex, statusResponse.indexOf(",", satellitesUsedIndex)));
        Serial.print("Signal Quality: ");
        Serial.println(statusResponse.substring(signalQualityIndex, statusResponse.indexOf(",", signalQualityIndex)));
      } else {
        Serial.println("Negalima gauti palydovų informaciją.");
      }
    }
};

#endif
