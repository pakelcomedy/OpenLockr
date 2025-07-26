cmake_minimum_required(VERSION 3.10.2)
project(OpenLockrNative C)

# Enforce C99 standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

#-------------------------------------------------------------------------------
# Include directories for all project headers
#-------------------------------------------------------------------------------
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/crypto
    ${CMAKE_SOURCE_DIR}/src/storage
    ${CMAKE_SOURCE_DIR}/src/sync
    ${CMAKE_SOURCE_DIR}/src/utils
)
list(REMOVE_ITEM OPENLOCKR_SOURCES "${CMAKE_SOURCE_DIR}/src/crypto/aes.c")
#-------------------------------------------------------------------------------
# SQLite3 (amalgamated source file)
#-------------------------------------------------------------------------------
add_library(sqlite3 STATIC
    ${CMAKE_SOURCE_DIR}/src/storage/sqlite3.c
)

#-------------------------------------------------------------------------------
# Optional: OpenSSL support (libcrypto from NDK)
# Uncomment to use OpenSSL (make sure libcrypto.a is present)
#-------------------------------------------------------------------------------
# set(OPENSSL_ROOT   ${ANDROID_NDK}/sources/third_party/openssl)
# set(OPENSSL_LIBDIR ${OPENSSL_ROOT}/libs/${ANDROID_ABI})
# if(EXISTS ${OPENSSL_LIBDIR}/libcrypto.a)
#     add_library(openssl_crypto STATIC IMPORTED)
#     set_target_properties(openssl_crypto PROPERTIES
#         IMPORTED_LOCATION "${OPENSSL_LIBDIR}/libcrypto.a"
#     )
#     include_directories("${OPENSSL_ROOT}/include")
# else()
#     message(WARNING "OpenSSL libcrypto.a not found at ${OPENSSL_LIBDIR}, skipping OpenSSL linking.")
# endif()

#-------------------------------------------------------------------------------
# Gather all OpenLockr C source files (excluding sqlite3.c)
#-------------------------------------------------------------------------------
file(GLOB_RECURSE OPENLOCKR_SOURCES
    ${CMAKE_SOURCE_DIR}/src/*.c
)

# Remove sqlite3.c if it's included in OPENLOCKR_SOURCES (to avoid duplication)
list(REMOVE_ITEM OPENLOCKR_SOURCES ${CMAKE_SOURCE_DIR}/src/storage/sqlite3.c)

#-------------------------------------------------------------------------------
# Build the main shared library for JNI
#-------------------------------------------------------------------------------
add_library(openlockr SHARED
    ${OPENLOCKR_SOURCES}
)

#-------------------------------------------------------------------------------
# Find Android log library and link dependencies
#-------------------------------------------------------------------------------
find_library(log-lib log)

target_link_libraries(openlockr
    ${log-lib}
    sqlite3
    # openssl_crypto  # Uncomment if using OpenSSL
)
