file(GLOB ADAFRUIT_GFX_SOURCES "src/*.cpp")

idf_component_register(
    SRCS        ${ADAFRUIT_GFX_SOURCES}
    INCLUDE_DIRS "src"
    REQUIRES    arduino-esp32 adafruit_busio
)
