file(GLOB AsyncMqttClient_SRC async-mqtt-client/src/*.cpp)
file(GLOB AsyncTCP_SRC AsyncTCP/src/*.cpp)
# set(AsyncMqttClient_SRC 
#       async-mqtt-client/src/AsyncMqttClient.cpp
#       async-mqtt-client/src/AsyncMqttClient.cpp
# )
set(LIB_INCLUDE_DIRS 
      async-mqtt-client/src/
      async-mqtt-client/src/AsyncMqttClient
      async-mqtt-client/src/AsyncMqttClient/Packets
      AsyncTCP/src/
      ESPAsyncTCP/src/
    )
set(LIB_SRCS ${AsyncMqttClient_SRC} ${AsyncTCP_SRC})

idf_component_register(SRCS ${LIB_SRCS}
                    INCLUDE_DIRS ${LIB_INCLUDE_DIRS}
                    REQUIRES arduino)