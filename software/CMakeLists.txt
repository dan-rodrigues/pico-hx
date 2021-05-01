cmake_minimum_required(VERSION 3.12)

include(pico_sdk_import.cmake)

project(picohx_demo C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(picohx_demo
        main.c
	usb_descriptors.c
	usb_uart.c
	usb_programmer.c
	fpga_spi.c
	get_serial.c
        )

pico_generate_pio_header(picohx_demo ${CMAKE_CURRENT_LIST_DIR}/spi.pio)

# Pull in our pico_stdlib which pulls in commonly used features
target_link_libraries(picohx_demo
	pico_stdlib
	hardware_pio
	tinyusb_device
	tinyusb_board
	pico_unique_id
)

target_include_directories(picohx_demo PRIVATE .)

# create map/bin/hex file etc.
pico_add_extra_outputs(picohx_demo)

