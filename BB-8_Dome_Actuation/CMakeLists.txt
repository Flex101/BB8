# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH "/home/matthew/RaspPi_Pico/pico-sdk")

set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.4.0")
	message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.4.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

project(BB-8_Dome_Actuation C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/externals/Pico-Servo-main/servo)

set(${PROJECT_NAME}_HDR
	include/Common.h
	include/MPU6050.h
	include/DomeMixer.h
	include/Servo.h
	include/Calibration.h
)

set(${PROJECT_NAME}_SRC
	src/main.cpp
	src/MPU6050.cpp
	src/DomeMixer.cpp
	src/Servo.cpp
	src/Calibration.cpp
)

# Add executable. Default name is the project name, version 0.1
add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_HDR} ${${PROJECT_NAME}_SRC})

pico_set_program_name(${PROJECT_NAME} "BB-8 Dome Actuation")
pico_set_program_version(${PROJECT_NAME} "0.1")

pico_enable_stdio_uart(${PROJECT_NAME} 1)
pico_enable_stdio_usb(${PROJECT_NAME} 1)

target_link_libraries(${PROJECT_NAME}
	pico_stdlib
	hardware_i2c
	hardware_pwm
	hardware_clocks
	pico_servo_object
)

target_include_directories(${PROJECT_NAME}
	PRIVATE
	${CMAKE_CURRENT_LIST_DIR}
	${CMAKE_CURRENT_LIST_DIR}/include
	${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts or any other standard includes, if required
)

pico_add_extra_outputs(${PROJECT_NAME})

add_compile_options(
	-Wall
	-Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
	-Wno-unused-function # we have some for the docs that aren't called
	-Wno-maybe-uninitialized
)