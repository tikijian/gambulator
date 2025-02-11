cmake_minimum_required(VERSION 3.16)
project(gambulator)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# This assumes the SDL source is available in vendored/SDL
add_subdirectory(libs/SDL3 EXCLUDE_FROM_ALL)
# set(CMAKE_C_STANDARD 11)

set(SOURCE_FILES
    src/software/utils.c
    src/hardware/cpu.c
    src/hardware/memory.c
    src/hardware/cpu_helpers.c
    src/hardware/timer.c
    src/hardware/clock.c
    src/main.c
)

# Create your game executable target as usual
add_executable(
    gambulator
    ${SOURCE_FILES}
)

# Link to the actual SDL3 library.
target_link_libraries(gambulator PRIVATE SDL3::SDL3)
# target_compile_options(my_c_program PRIVATE -Wall -Wextra)