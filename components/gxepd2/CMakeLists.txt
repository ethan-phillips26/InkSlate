# components/gxepd2/CMakeLists.txt

# Grab all .cpp files under src (including subfolders)
file(GLOB_RECURSE GXEPD2_SOURCES
    "src/*.cpp"
)

idf_component_register(
    SRCS
        ${GXEPD2_SOURCES}
    INCLUDE_DIRS
        "."
        "src"
    REQUIRES
        arduino-esp32
        adafruit_gfx
)
