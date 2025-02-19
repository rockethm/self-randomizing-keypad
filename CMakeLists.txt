# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(self-randomizing-keypad C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(self-randomizing-keypad self-randomizing-keypad.c )

pico_set_program_name(self-randomizing-keypad "self-randomizing-keypad")
pico_set_program_version(self-randomizing-keypad "0.1")

target_sources(self-randomizing-keypad PRIVATE self-randomizing-keypad.c ssd1306/ssd1306.c)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(self-randomizing-keypad 0)
pico_enable_stdio_usb(self-randomizing-keypad 1)

# Add the standard library to the build
target_link_libraries(self-randomizing-keypad
    pico_stdlib
    hardware_pio
    hardware_adc
    hardware_pwm
    hardware_i2c
    pico_time
    pico_rand
    pico_multicore
    pico_unique_id
    hardware_clocks
    hardware_timer
    hardware_sync
    hardware_irq
)

# Add the standard include files to the build
target_include_directories(self-randomizing-keypad PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(self-randomizing-keypad 
        
        )

pico_add_extra_outputs(self-randomizing-keypad)

