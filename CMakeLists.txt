cmake_minimum_required(VERSION 3.15)
project(capsicain VERSION 0.97.0 LANGUAGES CXX)

# Set C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Option to build tests (enabled by default)
option(BUILD_TESTING "Build unit tests" ON)

# Restrict build to only Visual Studio (MSVC)
if(NOT MSVC)
    message(FATAL_ERROR "Only Visual Studio (MSVC) is supported for building this project.")
endif()

# Set output directories to be inside build folder - using relative paths
if(CMAKE_BINARY_DIR MATCHES ".*/capsicain/build/.*")
    # We're already in a build subdirectory, so set output to current binary dir
    set(OUTPUT_DIR ${CMAKE_BINARY_DIR})
else()
    # Set output to build folder
    set(OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/capsicain/build)
endif()

# Set standard output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR})

# For multi-config generators (like Visual Studio)
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${OUTPUT_DIR}/${OUTPUTCONFIG})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${OUTPUT_DIR}/${OUTPUTCONFIG})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${OUTPUT_DIR}/${OUTPUTCONFIG})
endforeach()

# Set character set
add_compile_definitions(_MBCS)

# Define source files
set(CAPSICAIN_SOURCES
    capsicain/capsicain.cpp
    capsicain/led.cpp
    capsicain/modifiers.cpp
    capsicain/pch.cpp
    capsicain/scancodes.cpp
    capsicain/configUtils.cpp
    capsicain/traybar.cpp
    capsicain/utils.cpp
)

set(CAPSICAIN_HEADERS
    capsicain/capsicain.h
    capsicain/configUtils.h
    capsicain/constants.h
    capsicain/led.h
    capsicain/traybar.h
    capsicain/interception.h
    capsicain/modifiers.h
    capsicain/pch.h
    capsicain/resource.h
    capsicain/scancodes.h
    capsicain/utils.h
)

# Resource files handling for MSVC
set(CAPSICAIN_RESOURCES
    capsicain/capsicain.rc
)

# Define the executable
add_executable(capsicain ${CAPSICAIN_SOURCES} ${CAPSICAIN_HEADERS} ${CAPSICAIN_RESOURCES})

# Include directories
target_include_directories(capsicain PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/capsicain
    ${CMAKE_CURRENT_SOURCE_DIR}/capsicain/interception
)

# Diagnostic information
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME} - ${CMAKE_SIZEOF_VOID_P}bit")

# Precompiled headers
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.16")
    target_precompile_headers(capsicain PRIVATE capsicain/pch.h)
    set_source_files_properties(capsicain/utils.cpp PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
endif()

# Find the interception library (lib or dll)
set(INTERCEPTION_LIB "${CMAKE_CURRENT_SOURCE_DIR}/capsicain/interception.lib")
set(INTERCEPTION_DLL "${CMAKE_CURRENT_SOURCE_DIR}/capsicain/interception.dll")

# MSVC specific flags
target_compile_options(capsicain PRIVATE /W3)
target_compile_definitions(capsicain PRIVATE _CRT_SECURE_NO_WARNINGS)

# Debug vs Release specific settings
target_compile_definitions(capsicain 
    PRIVATE 
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

# Link libraries
target_link_libraries(capsicain
    # Core Windows libraries
    user32
    gdi32
    advapi32
    shell32
    ole32
    oleaut32
    uuid
    winspool
    comdlg32
    # Additional libraries
    setupapi
)

# Dynamic linking of interception.dll
message(STATUS "Using dynamic linking for interception.dll")

target_link_libraries(capsicain ${INTERCEPTION_LIB})

# Always copy the interception.dll, capsicain.ini, and AutoHotKey64.dll to the output directory
add_custom_command(TARGET capsicain POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${INTERCEPTION_DLL}"
        $<TARGET_FILE_DIR:capsicain>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/capsicain/capsicain.ini"
        $<TARGET_FILE_DIR:capsicain>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/capsicain/AutoHotKey64.dll"
        $<TARGET_FILE_DIR:capsicain>
    COMMENT "Copying interception.dll, capsicain.ini, and AutoHotKey64.dll to output directory"
)

# Update installation rules to include interception.dll
install(TARGETS capsicain DESTINATION .)
install(FILES 
    "${CMAKE_CURRENT_SOURCE_DIR}/capsicain/capsicain.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/capsicain/AutoHotKey64.dll"
    "${CMAKE_CURRENT_SOURCE_DIR}/capsicain/interception.dll"
    "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
    DESTINATION .
)

# ============================================================================
# Testing Support (Google Test)
# ============================================================================
if(BUILD_TESTING)
    # Enable testing
    enable_testing()

    # Fetch Google Test
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    # Add tests subdirectory
    add_subdirectory(tests)

    message(STATUS "Unit tests enabled. Run with 'ctest' or 'ctest --output-on-failure'")
endif()