cmake_minimum_required(VERSION 3.12)
set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
set(PICO_SDK_INIT_CMAKE_FILE ${PICO_SDK_PATH}/pico_sdk_init.cmake)
include(${PICO_SDK_INIT_CMAKE_FILE})
project(example_program LANGUAGES C CXX)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()
add_compile_options(-Wall -Wno-format -Werror -O3 -g0 -fdata-sections -ffunction-sections -Wl,--gc-sections)
include_directories(src/include)
include_directories(src/include/pico_usb_console/config)
add_executable(example_program
	src/main.c
	src/pico_usb_console/descriptor.c
	src/pico_usb_console/protocol.c
	src/pico_usb_console/serial.c
)
pico_enable_stdio_usb(example_program 0)
pico_enable_stdio_uart(example_program 0)
target_link_libraries(example_program pico_stdlib tinyusb_device tinyusb_board)
pico_add_extra_outputs(example_program)
