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
│       ├── async-mqtt-client # async-mqtt-client from https://github.com/ivanseidel/async-mqtt-client
│       ├── AsyncTCP          # AsyncTCP from https://github.com/ivanseidel/AsyncTCP
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
