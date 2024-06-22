cmake_minimum_required(VERSION 3.12)
set(PICO_BOARD pico_w)
# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

#include(pico_extras_import_optional.cmake)

project(pico_bt_guitar C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

set(PICO_EXAMPLES_PATH ${PROJECT_SOURCE_DIR})

# Initialize the SDK
pico_sdk_init()

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        )
if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wno-maybe-uninitialized)
endif()

set(BTSTACK_ROOT ${PICO_SDK_PATH}/lib/btstack)
set(BTSTACK_EXAMPLE_PATH ${BTSTACK_ROOT}/example)


# Adds common stuff for all the examples
add_library(picow_bt_common INTERFACE)
target_sources(picow_bt_common INTERFACE
        #${CMAKE_CURRENT_LIST_DIR}/config/picow_bt_example_common.c
        #${CMAKE_CURRENT_LIST_DIR}/config/picow_bt_example_background.c
        ${CMAKE_CURRENT_LIST_DIR}/src/blue_hid.c
        ${CMAKE_CURRENT_LIST_DIR}/src/main.c
        )
target_link_libraries(picow_bt_common INTERFACE
        hardware_adc
        pico_stdlib
        pico_btstack_cyw43
        pico_cyw43_arch_threadsafe_background
        )
target_include_directories(picow_bt_common INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/src/config # Use our own config
        #${BTSTACK_EXAMPLE_PATH}/
        )
target_compile_definitions(picow_bt_common INTERFACE
        PICO_STDIO_USB_CONNECT_WAIT_TIMEOUT_MS=3000
        CYW43_LWIP=0
        #WANT_HCI_DUMP=1 # This enables btstack debug
        #ENABLE_SEGGER_RTT=1
        )

add_subdirectory(src/flash)

add_executable(${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} PRIVATE
        picow_bt_common
        pico_btstack_ble
        pico_btstack_classic
        flash
)

#pico_enable_stdio_usb(${PROJECT_NAME} 1)
#pico_enable_stdio_uart(${PROJECT_NAME} 0)
pico_add_extra_outputs(${PROJECT_NAME})

