#include "Arduino.h"
#include "FS.h"
#include "esp_system.h"
#include <esp_wifi.h>
#include <string.h>
#include <WiFi.h>
#include <Preferences.h> // WiFi storage
#include <PubSubClient.h>

int ClearWifiPin = 4;

const char *rssiSSID; // NO MORE hard coded set AP, all SmartConfig
const char *password;
String PrefSSID, PrefPassword; // used by preferences storage

int WFstatus;
int UpCount = 0;
int32_t rssi; // store WiFi signal strength here
String getSsid;
String getPass;
String MAC;

WiFiClient espClient;
PubSubClient client(espClient);
const char* mqtt_server = "192.168.123.46";
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

// SSID storage
Preferences preferences; // declare class object
// END SSID storage

void wifiInit();
bool checkPrefsStore();
void initSmartConfig();
void IP_info();
int getWifiStatus(int WiFiStatus);
int32_t getRSSI(const char *target_ssid);
String getMacAddress(void);
String getSsidPass(String s);

void wifiInit() //
{
  WiFi.mode(WIFI_AP_STA); // required to read NVR before WiFi.begin()

  // load credentials from NVR, a little RTOS code here
  wifi_config_t conf;
  esp_wifi_get_config(WIFI_IF_STA, &conf); // load wifi settings to struct comf
  rssiSSID = reinterpret_cast<const char *>(conf.sta.ssid);
  password = reinterpret_cast<const char *>(conf.sta.password);

  // Open Preferences with "wifi" namespace. Namespace is limited to 15 chars
  preferences.begin("wifi", false);
  PrefSSID = preferences.getString("ssid", "none");         // NVS key ssid
  PrefPassword = preferences.getString("password", "none"); // NVS key password
  preferences.end();

  // keep from rewriting flash if not needed
  if (!checkPrefsStore())   // see is NV and Prefs are the same
  {                         // not the same, setup with SmartConfig
    if (PrefSSID == "none") // New...setup wifi
    {
      initSmartConfig();
      delay(3000);
      ESP.restart(); // reboot with wifi configured
    }
  }

  // I flash LEDs while connecting here

  WiFi.begin(PrefSSID.c_str(), PrefPassword.c_str());

  int WLcount = 0;
  while (WiFi.status() != WL_CONNECTED && WLcount < 200) // can take > 100 loops depending on router settings
  {
    delay(100);
    Serial.printf(".");
    ++WLcount;
  }
  delay(3000);

  //  stop the led flasher here

} // END wifiInit()

// match WiFi IDs in NVS to Pref store,  assumes WiFi.mode(WIFI_AP_STA);  was executed
bool checkPrefsStore()
{
  bool val = false;
  String NVssid, NVpass, prefssid, prefpass;

  NVssid = getSsidPass("ssid");
  NVpass = getSsidPass("pass");

  // Open Preferences with my-app namespace. Namespace name is limited to 15 chars
  preferences.begin("wifi", false);
  prefssid = preferences.getString("ssid", "none");     // NVS key ssid
  prefpass = preferences.getString("password", "none"); // NVS key password
  preferences.end();

  if (NVssid.equals(prefssid) && NVpass.equals(prefpass))
  {
    val = true;
  }

  return val;
}

// optionally call this function any way you want in your own code
// to remap WiFi to another AP using SmartConfig mode.   Button, condition etc..
void initSmartConfig()
{
  // start LED flasher
  int loopCounter = 0;

  WiFi.mode(WIFI_AP_STA); // Init WiFi, start SmartConfig
  Serial.printf("Entering SmartConfig\n");

  WiFi.beginSmartConfig();

  while (!WiFi.smartConfigDone())
  {
    // flash led to indicate not configured
    Serial.printf(".");
    if (loopCounter >= 40) // keep from scrolling sideways forever
    {
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

  while (WiFi.status() != WL_CONNECTED) // check till connected
  {
    delay(500);
  }
  IP_info(); // connected lets see IP info

  preferences.begin("wifi", false); // put it in storage
  preferences.putString("ssid", getSsid);
  preferences.putString("password", getPass);
  preferences.end();

  delay(300);
} // END SmartConfig()

void IP_info()
{
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

int getWifiStatus(int WiFiStatus)
{
  WiFiStatus = WiFi.status();
  Serial.printf("\tStatus %d", WiFiStatus);
  switch (WiFiStatus)
  {
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
// END getWifiStatus()

// Get the station interface MAC address.
// @return String MAC
String getMacAddress(void)
{
  WiFi.mode(WIFI_AP_STA); // required to read NVR before WiFi.begin()
  uint8_t baseMac[6];
  esp_read_mac(baseMac, ESP_MAC_WIFI_STA); // Get MAC address for WiFi station
  char macStr[18] = {0};
  sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
  return String(macStr);
}
// END getMacAddress()

// Return RSSI or 0 if target SSID not found
// const char* SSID = "YOUR_SSID";  // declare in GLOBAL space
// call:  int32_t rssi = getRSSI( SSID );
int32_t getRSSI(const char *target_ssid)
{
  byte available_networks = WiFi.scanNetworks();

  for (int network = 0; network < available_networks; network++)
  {
    if (strcmp(WiFi.SSID(network).c_str(), target_ssid) == 0)
    {
      return WiFi.RSSI(network);
    }
  }
  return 0;
} //  END  getRSSI()

// Requires; #include <esp_wifi.h>
// Returns String NONE, ssid or pass arcording to request
// ie String var = getSsidPass( "pass" );
String getSsidPass(String s)
{
  String val = "NONE"; // return "NONE" if wrong key sent
  s.toUpperCase();
  if (s.compareTo("SSID") == 0)
  {
    wifi_config_t conf;
    esp_wifi_get_config(WIFI_IF_STA, &conf);
    val = String(reinterpret_cast<const char *>(conf.sta.ssid));
  }
  if (s.compareTo("PASS") == 0)
  {
    wifi_config_t conf;
    esp_wifi_get_config(WIFI_IF_STA, &conf);
    val = String(reinterpret_cast<const char *>(conf.sta.password));
  }
  return val;
}

void setup()
{
  Serial.begin(115200);
  preferences.begin("wifi", false);
  bool isSmartConfig = digitalRead(ClearWifiPin);
  if (isSmartConfig == true)
  {
    Serial.println("clear config");
    preferences.clear();
  }
  preferences.end();

  Serial.printf("\tWiFi Setup -- \n");

  wifiInit(); // get WiFi connected
  IP_info();
  MAC = getMacAddress();
  Serial.println(" ");

  delay(3000);
  client.setServer(mqtt_server, 1883);
} //  END setup()

void reconnect() {
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    if (client.connect("ESP32_clientID")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "Nodemcu connected to MQTT");
      // ... and resubscribe
      client.subscribe("inTopic");

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    if (!client.connected()) {
      reconnect();
    }
    client.loop();

    unsigned long now = millis();
    if (now - lastMsg > 2000) {
      lastMsg = now;
      ++value;
      snprintf (msg, MSG_BUFFER_SIZE, "hello world #%d", value);
      Serial.print("Publish message: ");
      Serial.println(msg);
      client.publish("outTopic", msg);
    }
  } // END Main connected loop()
  else
  {
    // WiFi DOWN
    //  wifi down start LED flasher here
    WFstatus = getWifiStatus(WFstatus);
    WiFi.begin(PrefSSID.c_str(), PrefPassword.c_str());
    int WLcount = 0;
    while (WiFi.status() != WL_CONNECTED && WLcount < 200)
    {
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

    if (getWifiStatus(WFstatus) == 3)
    {
      // stop LED flasher, wifi going up
    }
    else if (getWifiStatus(WFstatus) == 6)
    {

      // Should delete it if button is pressed
      // initSmartConfig();
      delay(3000);
      ESP.restart(); // reboot with wifi configured
    }
    delay(1000);
  } // END WiFi down
}
// END loop()
