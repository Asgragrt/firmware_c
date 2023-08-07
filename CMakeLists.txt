# Set minimum required version of CMake
cmake_minimum_required(VERSION 3.13)

# Include build functions from Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# Set name of profect (as PROJECT_NAME) and C/C++ standards
project(firmware C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

#Creates a pico-sdk subdirectory in our project for the libraries
pico_sdk_init(cmake -DPICO_COPY_TO_RAM=1)

# Tell CMake where to find the executable source file
add_executable(${PROJECT_NAME}
    src/main.c
    src/tusb/tud_utils.c 
    src/kbd/kbd.c
    src/flash/flash.c
    src/leds/leds.c 
    src/leds/leds_utils.c
)

target_sources(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/src/tusb/usb_descriptors.c)

# Make sure TinyUSB can find tusb_config.h
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/src)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(${PROJECT_NAME})

# Link to pico_stdlib (gpio, time, etc. functions)
target_link_libraries(${PROJECT_NAME}
    pico_stdlib 
    tinyusb_device 
    tinyusb_board 
    hardware_pwm
    pico_multicore
    hardware_flash
)

#pico_set_binary_type(TARGET copy_to_ram)