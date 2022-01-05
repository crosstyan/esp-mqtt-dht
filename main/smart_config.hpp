#pragma once

#include "Arduino.h"
#include "FS.h"
#include "esp_system.h"
#include <Preferences.h> // WiFi storage
#include <WiFi.h>
#include <esp_wifi.h>
#include <string.h>

// All defined in .cpp files
extern const int CLEAR_WIFI_PIN;

extern const char *PREF_NAME;
extern const char *rssiSSID; // NO MORE hard coded set AP, all SmartConfig
extern const char *password;
extern String PrefSSID, PrefPassword; // used by preferences storage

extern int WiFiStatus;
extern int UpCount;
extern int32_t rssi; // store WiFi signal strength here
extern String WiFiSSID;
extern String WiFiPassword;
extern String MAC;
extern uint8_t ReservedData[128];

// SSID storage
extern Preferences preferences; // declare class object
// END SSID storage

void WiFiInit();
bool checkPrefsStore();
void initSmartConfig();
void ipInfo();
void handleWiFiDown();
int getWifiStatus(int WiFiStatus);
int32_t getRSSI(const char *target_ssid);
String getMacAddress(void);
String getSsidPass(String s);