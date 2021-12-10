
set(MAIN_SRCS main.cpp smart_config.cpp)
idf_component_register(SRCS ${MAIN_SRCS}
                    INCLUDE_DIRS "."
                    REQUIRES arduino-libs)
