# Note that this file can be used as a top level CMakeLists.txt to build all the Demos (it includes
# them all, but they are usable in their own right, which is why no common work is done here)

cmake_minimum_required(VERSION 3.13)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

# Pull in FreeRTOS
include(FreeRTOS_Kernel_import.cmake)

project(synth C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(synth
        src/main.c
        src/log_task.c
        src/midi_task.c
        src/audio_task.c
        src/i2s.c
        )

pico_generate_pio_header(synth ${CMAKE_CURRENT_LIST_DIR}/src/i2s.pio)

target_include_directories(synth PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/src
        ${CMAKE_CURRENT_LIST_DIR}/../../Common/include)

target_link_libraries(synth 
        pico_stdlib
        hardware_dma
        hardware_pio
        hardware_clocks
        FreeRTOS-Kernel 
        FreeRTOS-Kernel-Heap4
        )

pico_add_extra_outputs(synth)
