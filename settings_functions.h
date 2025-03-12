#ifndef SETTINGS_FUNCTIONS_H
#define SETTINGS_FUNCTIONS_H

#include <ArduinoJson.h> // Reikalinga JSON tvarkymui
#include <FS.h>          // SPIFFS failų sistema
#include <SPIFFS.h>      // SPIFFS failų sistema

class Settings {
  private:
    const char* settingsFile = "/settings.txt"; // Failo kelias SPIFFS sistemoje
    StaticJsonDocument<1024> jsonDoc;           // JSON dokumentas nustatymų laikymui

  public:
    Settings() {
      if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS mount failed!");
      }
    }

    // Funkcija nuskaito nustatymus iš failo
    bool loadSettings() {
      File file = SPIFFS.open(settingsFile, "r");
      if (!file) {
        Serial.println("Failed to open settings file for reading");
        return false;
      }

      // Nuskaito JSON turinį iš failo
      DeserializationError error = deserializeJson(jsonDoc, file);
      file.close();

      if (error) {
        Serial.print("Failed to read file, using default configuration. Error: ");
        Serial.println(error.c_str());
        return false;
      }

      Serial.println("Settings loaded successfully");
      return true;
    }

    // Funkcija įrašo nustatymus į failą
    bool saveSettings() {
      File file = SPIFFS.open(settingsFile, "w");
      if (!file) {
        Serial.println("Failed to open settings file for writing");
        return false;
      }

      // Išsaugo JSON turinį į failą
      if (serializeJson(jsonDoc, file) == 0) {
        Serial.println("Failed to write to file");
        file.close();
        return false;
      }

      file.close();
      Serial.println("Settings saved successfully");
      return true;
    }

    // Funkcija, skirta atnaujinti bool tipo nustatymus
    void updateSettingBool(const char* key, bool value) {
      jsonDoc[key] = value;
    }

    // Funkcija, skirta atnaujinti string tipo nustatymus
    void updateSettingString(const char* key, const char* value) {
      jsonDoc[key] = value;
    }

    // Funkcija, skirta atnaujinti string masyvą
    void updateSettingArray(const char* key, const char* values[], size_t size) {
      JsonArray arr = jsonDoc.createNestedArray(key);
      for (size_t i = 0; i < size; i++) {
        arr.add(values[i]);
      }
    }

    // Funkcija, skirta nuskaityti bool nustatymą
    bool getSettingBool(const char* key) {
      return jsonDoc[key] | false;
    }

    // Funkcija, skirta nuskaityti string nustatymą
    const char* getSettingString(const char* key) {
      return jsonDoc[key] | "Not Found";
    }

    // Funkcija, skirta nuskaityti string masyvą
    void getSettingArray(const char* key, JsonArray& outputArray) {
      outputArray = jsonDoc[key].as<JsonArray>();
    }

    // Inicializuojame laukus su pradinėmis reikšmėmis, jei reikia
    void initializeDefaults() {
      if (!jsonDoc.containsKey("GPSpower")) updateSettingBool("GPSpower", true);
      if (!jsonDoc.containsKey("LTEPower")) updateSettingBool("LTEPower", true);
      if (!jsonDoc.containsKey("BlueToothPower")) updateSettingBool("BlueToothPower", true);
      if (!jsonDoc.containsKey("ServerAddress")) updateSettingString("ServerAddress", "https://gps.freya.lt/");
      if (!jsonDoc.containsKey("SaibaID")) updateSettingString("SaibaID", "DefaultID");
      if (!jsonDoc.containsKey("LTEPowerTimers")) {const char* defaultTimers[] = {"00:00,20", "06:00,20", "12:00,20", "18:00,20"};
        updateSettingArray("LTEPowerTimers", defaultTimers, 4);
      }
    }
};

#endif
