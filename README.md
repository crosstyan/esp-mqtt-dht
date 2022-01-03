# _Sample project_

(See the README.md file in the upper level 'examples' directory for more information about examples.)

This is the simplest buildable example. The example is used by command `idf.py create-project`
that copies the project to user specified path and set it's name. For more information follow the [docs page](https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html#start-a-new-project)

## Structure

```txt
.
├── CMakeLists.txt
├── components
│   ├── arduino         # Arduino components from https://github.com/espressif/arduino-esp32
│   ├── ArduinoJson     # ArduinoJson from https://github.com/bblanchon/ArduinoJson
│   └── arduino-libs    # Other Arduino libs
│       ├── Adafruit_Sensor   # Adafruit_Sensor from https://github.com/adafruit/Adafruit_Sensor
│       ├── async-mqtt-client # async-mqtt-client from https://github.com/ivanseidel/async-mqtt-client  (which is not used anymore)
│       ├── AsyncTCP          # AsyncTCP from https://github.com/ivanseidel/AsyncTCP                    (which is not used anymore)
│       ├── CMakeLists.txt
│       ├── DHT-sensor-library # DHT-sensor-library from https://github.com/adafruit/DHT-sensor-library
│       └── pubsubclient       # pubsubclient from https://github.com/knolleary/pubsubclient
├── main
│   ├── CMakeLists.txt
│   ├── main.cpp
│   ├── smart_config.cpp
│   ├── smart_config.h
│   └── smart_config.hpp
├── README.md
└── sdkconfig
```

## Getting started

I assume that working ESP-IDF Tools is installed. Check [Get Started](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/) for more information.

### Build

```bash
idf.py build
```

### Flash

I assume that the device (ESP32) is connected to COM9.

```bash
idf.py -p COM9 flash
```

## Usage

### SmartConfig

I made some modifications to [arduino-esp32
](https://github.com/crosstyan/arduino-esp32/commit/cd44f4a5773da007b0b8997f11f5c2e1772a2202) to make it work with ESPTOUCH v2 in
order to use the reserved filed, which means that the ESPTOUCH v1 is not available anymore.

User's custom data, the max length is 127. The configuration should be easier now with reserved field. Check [EspressifApp/lib-esptouch-v2-android](https://github.com/EspressifApp/lib-esptouch-v2-android/tree/main/esptouch-v2) for more information.

The device is configured to connect to the AP with [SmartConfig](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_smartconfig.html)
and SSID and password are saved in the device after the first connection.

You can clear these information by connect the `ClearWifiPin` defined in `smart_config.hpp` (defualt GPIO 4).

Use [ESP-TOUCH](https://www.espressif.com/en/products/software/esp-touch/resources) to configure the device.

### DHT sensor

The DHT sensor is connected to the GPIO 2 by default, which can be changed in `main.cpp`.

### MQTT Server

Temperature and humidity are published to the MQTT server with the topic `temperature` and `humidity`. Change the MQTT server address and port in `main.cpp` if needed.

## How to add Arduino libraries

Move the library you need to `components/arduino-libs` directory and then
edit `CMakeLists.txt`.

Add all the `cpp` files in your library directory to `LIB_SRCS` varible in `CMakeLists.txt`. Don't forget to add inlude directories to `LIB_INCLUDE_DIRS`.

## Useful Links

- [esp-idf/examples/wifi/smart_config at espressif/esp-idf](https://github.com/espressif/esp-idf/tree/1cb31e50943bb757966ca91ed7f4852692a5b0ed/examples/wifi/smart_config)