cmake_minimum_required(VERSION 3.16)
project(gambulator)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")


set(SOURCE_FILES
    src/software/utils.c
    src/hardware/cpu.c
    src/hardware/memory.c
    src/hardware/cpu_helpers.c
    src/hardware/timer.c
    src/hardware/clock.c
    src/hardware/interrupts.c
    src/hardware/ppu.c
    src/main.c
)

# Create your game executable target as usual
add_executable(
    gambulator
    ${SOURCE_FILES}
)
target_compile_options(gambulator PRIVATE -Wall -Wextra)

add_executable(raylib_main src/raylib_main.c)
target_link_libraries(raylib_main PUBLIC raylib m)
# target_compile_options(my_c_program PRIVATE -Wall -Wextra)