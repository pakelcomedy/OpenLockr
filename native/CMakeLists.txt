# native/CMakeLists.txt

cmake_minimum_required(VERSION 3.10.2)

project(OpenLockrNative C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Include directories
include_directories(
    src
    src/crypto
    src/storage
    src/sync
    src/utils
)

# Collect all .c files
file(GLOB_RECURSE SOURCES
    src/*.c
)

# Create shared library (.so)
add_library(
    openlockr SHARED
    ${SOURCES}
)

# Link any required native libraries (e.g., for logcat)
find_library(log-lib log)

target_link_libraries(
    openlockr
    ${log-lib}
)