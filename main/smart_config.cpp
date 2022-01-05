#ifndef SRC_SMART_CONFIG_H_
#define SRC_SMART_CONFIG_H_

#include "smart_config.hpp"

#endif // SRC_ASYNCMQTTCLIENT_H_

const int CLEAR_WIFI_PIN = 4;

const char *PREF_NAME = "wifi";
const char *rssiSSID; // NO MORE hard coded set AP, all SmartConfig
const char *password;
String PrefSSID, PrefPassword;
// used by preferences storage

int WFstatus;
int UpCount = 0;
int32_t rssi; // store WiFi signal strength here
String getSsid;
String getPass;

// User's custom data, nullable. If not null, the max length is 127
uint8_t getRvd[128] = {0};
String MAC;

Preferences preferences;

void WiFiInit() {
  WiFi.mode(WIFI_AP_STA); // required to read NVR before WiFi.begin()

  // load credentials from NVR, a little RTOS code here
  wifi_config_t conf;
  esp_wifi_get_config(WIFI_IF_STA, &conf); // load wifi settings to struct comf
  rssiSSID = reinterpret_cast<const char *>(conf.sta.ssid);
  password = reinterpret_cast<const char *>(conf.sta.password);

  // Open Preferences with "wifi" namespace. Namespace is limited to 15 chars
  preferences.begin(PREF_NAME, false);
  PrefSSID = preferences.getString("ssid", "none");         // NVS key ssid
  PrefPassword = preferences.getString("password", "none"); // NVS key password
  preferences.getBytes("rvd", getRvd, 128); // SmartConfig Reserved Data
  preferences.end();

  // keep from rewriting flash if not needed
  // see is NV and Prefs are the same
  if (!checkPrefsStore()) {
    // not the same, setup with SmartConfig
    // New...setup wifi
    if (PrefSSID == "none") {
      initSmartConfig();
      delay(3000);
      ESP.restart(); // reboot with wifi configured
    }
  }

  // I flash LEDs while connecting here

  WiFi.begin(PrefSSID.c_str(), PrefPassword.c_str());

  // Line wrap count
  int WLcount = 0;
  while (WiFi.status() != WL_CONNECTED &&
         WLcount < 200) // can take > 100 loops depending on router settings
  {
    delay(100);
    Serial.printf(".");
    ++WLcount;
  }
  delay(3000);

  //  stop the led flasher here

} // END WiFiInit()

// match WiFi IDs in NVS to Pref store,  assumes WiFi.mode(WIFI_AP_STA);  was
// executed
bool checkPrefsStore() {
  bool val = false;
  String NVssid, NVpass, prefssid, prefpass;

  NVssid = getSsidPass("ssid");
  NVpass = getSsidPass("pass");

  // Open Preferences with my-app namespace. Namespace name is limited to 15
  // chars
  preferences.begin(PREF_NAME, false);
  prefssid = preferences.getString("ssid", "none");     // NVS key ssid
  prefpass = preferences.getString("password", "none"); // NVS key password
  preferences.end();

  if (NVssid.equals(prefssid) && NVpass.equals(prefpass)) {
    val = true;
  }

  return val;
}

// optionally call this function any way you want in your own code
// to remap WiFi to another AP using SmartConfig mode.   Button, condition etc..
void initSmartConfig() {
  // start LED flasher
  int loopCounter = 0;

  WiFi.mode(WIFI_AP_STA); // Init WiFi, start SmartConfig
  Serial.printf("Entering SmartConfig\n");

  WiFi.beginSmartConfig();

  while (!WiFi.smartConfigDone()) {
    // flash led to indicate not configured
    Serial.printf(".");
    // keep from scrolling sideways forever
    if (loopCounter >= 40) {
      loopCounter = 0;
      Serial.printf("\n");
    }
    delay(600);
    ++loopCounter;
  }
  loopCounter = 0;

  // stopped flasher here
  Serial.printf("\nSmartConfig received.\n Waiting for WiFi\n\n");
  delay(2000);
  // check till connected
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  ipInfo(); // connected lets see IP info
  uint8_t *smartConfigRvdData = WiFi.smartConfigRvdData();
  memcpy(getRvd, smartConfigRvdData, sizeof(uint8_t) * 127);
  preferences.begin(PREF_NAME, false); // put it in storage
  preferences.putString("ssid", getSsid);
  preferences.putString("password", getPass);
  // preferences.putString("rvd", getRvd);
  preferences.putBytes("rvd", getRvd, 128);
  preferences.end();

  delay(300);
} // END SmartConfig()

void ipInfo() {
  getSsid = WiFi.SSID();
  getPass = WiFi.psk();
  rssi = getRSSI(rssiSSID);
  WFstatus = getWifiStatus(WFstatus);
  MAC = getMacAddress();

  Serial.printf("\n\n\tSSID\t%s, ", getSsid.c_str());
  Serial.print(rssi);
  Serial.printf(" dBm\n"); // printf??
  Serial.printf("\tPass:\t %s\n", getPass.c_str());
  Serial.print("\n\n\tIP address:\t");
  Serial.print(WiFi.localIP());
  Serial.print(" / ");
  Serial.println(WiFi.subnetMask());
  Serial.print("\tGateway IP:\t");
  Serial.println(WiFi.gatewayIP());
  Serial.print("\t1st DNS:\t");
  Serial.println(WiFi.dnsIP());
  Serial.printf("\tMAC:\t\t%s\n", MAC.c_str());
}

int getWifiStatus(int WiFiStatus) {
  WiFiStatus = WiFi.status();
  Serial.printf("\tStatus %d", WiFiStatus);
  switch (WiFiStatus) {
    case WL_IDLE_STATUS: // WL_IDLE_STATUS     = 0,
      Serial.printf(", WiFi IDLE \n");
      break;
    case WL_NO_SSID_AVAIL: // WL_NO_SSID_AVAIL   = 1,
      Serial.printf(", NO SSID AVAIL \n");
      break;
    case WL_SCAN_COMPLETED: // WL_SCAN_COMPLETED  = 2,
      Serial.printf(", WiFi SCAN_COMPLETED \n");
      break;
    case WL_CONNECTED: // WL_CONNECTED       = 3,
      Serial.printf(", WiFi CONNECTED \n");
      break;
    case WL_CONNECT_FAILED: // WL_CONNECT_FAILED  = 4,
      Serial.printf(", WiFi WL_CONNECT FAILED\n");
      break;
    case WL_CONNECTION_LOST: // WL_CONNECTION_LOST = 5,
      Serial.printf(", WiFi CONNECTION LOST\n");
      WiFi.persistent(false); // don't write FLASH
      break;
    case WL_DISCONNECTED: // WL_DISCONNECTED    = 6
      Serial.printf(", WiFi DISCONNECTED ==\n");
      WiFi.persistent(false); // don't write FLASH when reconnecting
      break;
  }
  return WiFiStatus;
}

// @brief     Get the station interface MAC address.
// @param     void
// @return String MAC
String getMacAddress(void) {
  WiFi.mode(WIFI_AP_STA); // required to read NVR before WiFi.begin()
  uint8_t baseMac[6];
  esp_read_mac(baseMac, ESP_MAC_WIFI_STA); // Get MAC address for WiFi station
  char macStr[18] = {0};
  sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", baseMac[0], baseMac[1],
          baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
  return String(macStr);
}

// Return RSSI or 0 if target SSID not found
// const char* SSID = "YOUR_SSID";  // declare in GLOBAL space
// call:  int32_t rssi = getRSSI( SSID );
int32_t getRSSI(const char *target_ssid) {
  byte available_networks = WiFi.scanNetworks();

  for (int network = 0; network < available_networks; network++) {
    if (strcmp(WiFi.SSID(network).c_str(), target_ssid) == 0) {
      return WiFi.RSSI(network);
    }
  }
  return 0;
} //  END  getRSSI()

// Why the original guy use string to represent the SSID and password?
// Maybe it's not a bad choose though.
// TODO: Use enum instead
// Requires; #include <esp_wifi.h>
// Returns String NONE, ssid or pass arcording to request
// ie String var = getSsidPass( "pass" );
String getSsidPass(String s) {
  String val = "NONE"; // return "NONE" if wrong key sent
  s.toUpperCase();
  if (s.compareTo("SSID") == 0) {
    wifi_config_t conf;
    esp_wifi_get_config(WIFI_IF_STA, &conf);
    val = String(reinterpret_cast<const char *>(conf.sta.ssid));
  }
  if (s.compareTo("PASS") == 0) {
    wifi_config_t conf;
    esp_wifi_get_config(WIFI_IF_STA, &conf);
    val = String(reinterpret_cast<const char *>(conf.sta.password));
  }
  return val;
}

// WiFi DOWN
void handleWiFiDown() {
  //  wifi down start LED flasher here
  WFstatus = getWifiStatus(WFstatus);
  WiFi.begin(PrefSSID.c_str(), PrefPassword.c_str());
  int WLcount = 0;
  while (WiFi.status() != WL_CONNECTED && WLcount < 200) {
    delay(100);
    Serial.printf(".");

    if (UpCount >= 60) // keep from scrolling sideways forever
    {
      UpCount = 0;
      Serial.printf("\n");
    }
    ++UpCount;
    ++WLcount;
  }

  if (getWifiStatus(WFstatus) == 3) {
    // stop LED flasher, wifi going up
  } else if (getWifiStatus(WFstatus) == 6) {

    // Should delete it if button is pressed
    // initSmartConfig();
    delay(3000);
    ESP.restart(); // reboot with wifi configured
  }
  delay(1000);
}