idf_component_register(
    SRCS
        "ble_ancs.c"
        "ble_ancs_demo.c"
        "arduino_main.cpp"
        "notification_queue.c"
    INCLUDE_DIRS "."
    REQUIRES
        arduino-esp32
        gxepd2
)
