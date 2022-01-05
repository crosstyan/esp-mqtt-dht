#include "Arduino.h"
#include "FS.h"
#include "esp_event.h"
#include "esp_system.h"
#include "smart_config.h"
#include <DHT.h>
#include <Preferences.h> // WiFi storage
#include <PubSubClient.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <string.h>

#define DHTPIN 2      // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11 // DHT 11

DHT dht(DHTPIN, DHTTYPE);

WiFiClient espClient;
PubSubClient client(espClient);
char MQTT_HOST[60] = {0};
// default MQTT port is 1883
int MQTT_PORT = 1883;
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
int helloCount = 0;

/*
 * @brief clear preferences
 * @param void
 * @return void
 */
void clearPreferences(const char *name) {
  preferences.begin(name, false);
  preferences.clear();
  preferences.end();
}

// The reserved data should be host:port
// port number is mandatory.
// Only ipv4 is supported I guess.
/*
 * @brief  Parse the MQTT server address and port
 * @param address
 * @return err
 *   0: success
 *   -1: error
 */
int parseAddress(uint8_t *address) {
  char *colonCursor = strstr((char *) address, ":");
  // if address contains colon
  if (colonCursor) {
    MQTT_PORT = atoi(colonCursor + 1);
    if (MQTT_PORT < 1 || MQTT_PORT > 65535) {
      // indicate invalid port
      return -1;
    }
    // not include colon
    memcpy(MQTT_HOST, address, (uint8_t *) colonCursor - address);
  } else {
    return -1;
  }
  return 0;
}

void printHex(char * str){
  // Print out Reserved Data
  Serial.println((char *) str);
  // Print out Reserved Data as Hex if it's not empty
  char *cursor = (char *) str;
  while (*cursor) {
    Serial.printf("%02x ", *cursor);
    cursor++;
  }
  Serial.printf("\n");
}

void setup() {
  Serial.begin(115200);
  // Clear the preferences if the pin is high
  bool isSmartConfig = digitalRead(CLEAR_WIFI_PIN);
  if (isSmartConfig) {
    Serial.println("clear config");
    clearPreferences(PREF_NAME);
  }

  Serial.printf("\tWiFi Setup -- \n");
  WiFiInit(); // get WiFi connected
  ipInfo();
  printHex((char *)getRvd);
  // end of Printing
  int err = parseAddress(getRvd);
  if (err) {
    Serial.println("Invalid address");
    clearPreferences(PREF_NAME);
  }
  MAC = getMacAddress();

  delay(3000);
  Serial.printf("MQTT Host: %s\n", MQTT_HOST);
  Serial.printf("MQTT Port: %d\n", MQTT_PORT);
  client.setServer(MQTT_HOST, MQTT_PORT);
  dht.begin();
} //  END setup()

void MQTT_reconnect() {
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    if (client.connect("ESP32_clientID")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

/*
 * @brief Read and Publish current temperature and humidity to MQTT broker
 * @param void
 * @return err
 *  0: success
 *  -1: failed
 */
int publishTmpHmd() {
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  char hMsg[20];
  char tMsg[20];

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return -1;
  }
  snprintf(hMsg, 20, "%.1f", h);
  snprintf(tMsg, 20, "%.1f", t);

  client.publish("temperature", tMsg);
  client.publish("humidity", hMsg);
  return 0;
}

int publishHello(int count) {
  snprintf(msg, MSG_BUFFER_SIZE, "hello world #%d", count);
  Serial.print("Publish message: ");
  Serial.println(msg);
  client.publish("hello", msg);
  return 0;
}


void loop() {
  if (likely(WiFi.status() == WL_CONNECTED)) {
    if (!client.connected()) {
      MQTT_reconnect();
    }
    client.loop();

    unsigned long now = millis();
    if (now - lastMsg > 2000) {
      lastMsg = now;
      ++helloCount;
      publishHello(helloCount);
      publishTmpHmd();
    }
  } // END Main connected loop()
  else {
    handleWiFiDown();
  }
}