# file(GLOB AsyncMqttClient_SRC async-mqtt-client/src/*.cpp)
# file(GLOB AsyncTCP_SRC AsyncTCP/src/*.cpp)
file(GLOB PubSubClient_SRC pubsubclient/src/*.cpp)
file(GLOB AdafruitSensor_SRC Adafruit_Sensor/*.cpp)
file(GLOB Adafruit_DHT_SRC DHT-sensor-library/*.cpp)

set(LIB_INCLUDE_DIRS 
      # async-mqtt-client/src/
      # async-mqtt-client/src/AsyncMqttClient
      # async-mqtt-client/src/AsyncMqttClient/Packets
      # AsyncTCP/src/
      pubsubclient/src/
      DHT-sensor-library/
      Adafruit_Sensor/
    )

set(LIB_SRCS 
  # ${AsyncMqttClient_SRC} 
  # ${AsyncTCP_SRC} 
  ${PubSubClient_SRC}
  ${Adafruit_Sensor_SRC}
  ${Adafruit_DHT_SRC}
)

idf_component_register(SRCS ${LIB_SRCS}
                      INCLUDE_DIRS ${LIB_INCLUDE_DIRS}
                      REQUIRES arduino)