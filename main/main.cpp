#include "Arduino.h"
#include "FS.h"
#include "esp_system.h"
#include <esp_wifi.h>
#include <string.h>
#include <WiFi.h>
#include <Preferences.h> // WiFi storage
#include "smart_config.h"
#include <PubSubClient.h>
#include <DHT.h>


WiFiClient espClient;
PubSubClient client(espClient);
const char* mqtt_server = "192.168.123.46";
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

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

void MQTT_reconnect() {
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
      MQTT_reconnect();
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
