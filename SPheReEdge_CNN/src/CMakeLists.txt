# CMakeLists.txt (dentro de src/)
idf_component_register(
    SRCS "main.cpp"
    INCLUDE_DIRS "."
    REQUIRES esp32-camera esp-tflite-micro esp_psram
)
