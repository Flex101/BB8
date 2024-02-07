# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH "/home/matthew/RaspPi_Pico/pico-sdk")

set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.4.0")
  message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.4.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

project(BB-8_Ball_Roll C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

set(${PROJECT_NAME}_HDR
	include/lwipopts.h
	include/btstack_config.h
	include/btController.h
)

set(${PROJECT_NAME}_SRC
	src/main.cpp
	#src/btstack/spp_counter.c
	src/btstack/spp_streamer.c
	src/btController.cpp
)

# Add executable. Default name is the project name, version 0.1

add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_HDR} ${${PROJECT_NAME}_SRC})

pico_set_program_name(${PROJECT_NAME} "BB-8 Ball Roll")
pico_set_program_version(${PROJECT_NAME} "0.1")

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 1)

# Add the standard library to the build
target_link_libraries(${PROJECT_NAME}
  pico_stdlib
  pico_btstack_classic
  pico_btstack_cyw43
  pico_cyw43_arch_none
  #pico_cyw43_arch_lwip_threadsafe_background
)

# Add the standard include files to the build
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

