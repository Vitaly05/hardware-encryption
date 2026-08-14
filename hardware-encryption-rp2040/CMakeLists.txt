# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialize pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
	set(USERHOME $ENV{USERPROFILE})
else()
	set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.3.0)
set(toolchainVersion 15_2_Rel1)
set(picotoolVersion 2.3.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
	include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD waveshare_rp2040_zero CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(hardware_encryption C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

# start libsodium -------
set(SODIUM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libsodium")

file(GLOB_RECURSE ALL_SODIUM_FILES 
	RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
	"libsodium/*.c"
)

set(SODIUM_SOURCES "")
foreach(SOURCE_FILE ${ALL_SODIUM_FILES})
	if(NOT SOURCE_FILE MATCHES "RECYCLE" AND
	   NOT SOURCE_FILE MATCHES "shorthash_siphashx24" AND 
	   NOT SOURCE_FILE MATCHES "xmm6int" AND
	   NOT SOURCE_FILE MATCHES "crypto_pwhash/scrypt" AND
	   NOT SOURCE_FILE MATCHES "crypto_auth/hmacsha" AND
	   NOT SOURCE_FILE MATCHES "crypto_aead/aes256")
		
		list(APPEND SODIUM_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}")
	endif()
endforeach()

add_library(sodium STATIC ${SODIUM_SOURCES})

target_compile_definitions(sodium PRIVATE 
	SODIUM_STATIC=1
	HAVE_CONFIG_H=1
	CONFIG_H=1
	SODIUM_LIBRARY_MINIMAL=1
)

target_include_directories(sodium PRIVATE 
	"${SODIUM_DIR}/include"
	"${SODIUM_DIR}/include/sodium"
)

# end libsodium -------

add_executable(${PROJECT_NAME}
	hardware_encryption.c
	usb_descriptors.c
	sodium_config.c
)

pico_set_program_name(${PROJECT_NAME} "hardware_encryption")
pico_set_program_version(${PROJECT_NAME} "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(${PROJECT_NAME} 0)
pico_enable_stdio_usb(${PROJECT_NAME} 0)

# Add the standard library to the build
target_link_libraries(${PROJECT_NAME}
	pico_stdlib
	pico_rand
	hardware_pio
	tinyusb_device
	tinyusb_board
	sodium
)

# Add the standard include files to the build
target_include_directories(${PROJECT_NAME} PRIVATE
	${CMAKE_CURRENT_LIST_DIR}
	${SODIUM_DIR}/include
)

pico_add_extra_outputs(${PROJECT_NAME})

pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)
