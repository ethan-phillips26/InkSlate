idf_component_register(
    SRCS
        "src/Adafruit_I2CDevice.cpp"
        "src/Adafruit_SPIDevice.cpp"
        "src/Adafruit_BusIO_Register.cpp"
    INCLUDE_DIRS
        "."
        "src"
    REQUIRES
        arduino-esp32
)

