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
if(CMAKE_BINARY_DIR MATCHES ".*/build/.*")
    # We're already in a build subdirectory, so set output to current binary dir
    set(OUTPUT_DIR ${CMAKE_BINARY_DIR})
else()
    # Set output to build folder at root
    set(OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build)
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

# Define source files - organized by component
set(CAPSICAIN_SOURCES
    # Main entry point
    main.cpp

    # Core implementation
    src/CapsicainImpl.cpp

    # Core (domain constants & types)
    src/core/modifiers.cpp
    src/core/scancodes.cpp

    # Configuration
    src/config/configUtils.cpp

    # Utilities
    src/utils/StringUtils.cpp

    # Platform (Windows-specific)
    src/platform/pch.cpp
    src/platform/led.cpp
    src/platform/traybar.cpp

    # Application layer
    src/app/Application.cpp
    src/commands/CommandHandler.cpp
    src/ui/ConsoleUI.cpp

    # Services
    src/services/UIService.cpp
    src/services/ErrorService.cpp
    src/services/ConfigurationService.cpp
    src/services/HardwareService.cpp
    src/services/AutoHotkeyService.cpp
    src/services/RuntimeStateService.cpp
    src/services/KeyProcessingService.cpp
)

set(CAPSICAIN_HEADERS
    # Core implementation
    src/CapsicainImpl.h

    # Core (domain constants & types)
    src/core/KeyEvent.h
    src/core/ProcessingContext.h
    src/core/Types.h
    src/core/modifiers.h
    src/core/scancodes.h

    # Configuration
    src/config/configUtils.h

    # Utilities
    src/utils/StringUtils.h

    # Platform headers
    src/platform/pch.h
    src/platform/constants.h
    src/platform/resource.h
    src/platform/interception.h
    src/platform/led.h
    src/platform/traybar.h

    # Application layer headers
    src/app/Application.h
    src/commands/CommandHandler.h
    src/ui/ConsoleUI.h

    # Services headers
    src/services/UIService.h
    src/services/ErrorService.h
    src/services/ProfilingService.h
    src/services/ConfigurationService.h
    src/services/HardwareService.h
    src/services/AutoHotkeyService.h
    src/services/RuntimeStateService.h
    src/services/KeyProcessingService.h

    # Domain headers
    src/domain/TapDetector.h
    src/domain/ModifierTracker.h
    src/domain/ComboMatcher.h
    src/domain/KeyMapper.h
    src/domain/KeyProcessor.h
    src/domain/LeaderKey.h
    src/domain/TapDance.h

    # Interfaces
    src/interfaces/IDriver.h
)

# Resource files handling for MSVC
set(CAPSICAIN_RESOURCES
    src/assets/capsicain.rc
)

# Define the executable
add_executable(capsicain ${CAPSICAIN_SOURCES} ${CAPSICAIN_HEADERS} ${CAPSICAIN_RESOURCES})

# Include directories - src is the root for all includes
target_include_directories(capsicain PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/platform
)

# Diagnostic information
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME} - ${CMAKE_SIZEOF_VOID_P}bit")

# Precompiled headers
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.16")
    target_precompile_headers(capsicain PRIVATE src/platform/pch.h)
    set_source_files_properties(src/utils/StringUtils.cpp PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
endif()

# Find the interception library (lib or dll)
set(INTERCEPTION_LIB "${CMAKE_CURRENT_SOURCE_DIR}/src/platform/interception.lib")
set(INTERCEPTION_DLL "${CMAKE_CURRENT_SOURCE_DIR}/src/platform/interception.dll")

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
        "${CMAKE_CURRENT_SOURCE_DIR}/src/assets/capsicain.ini"
        $<TARGET_FILE_DIR:capsicain>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/src/assets/AutoHotKey64.dll"
        $<TARGET_FILE_DIR:capsicain>
    COMMENT "Copying interception.dll, capsicain.ini, and AutoHotKey64.dll to output directory"
)

# Update installation rules to include interception.dll
install(TARGETS capsicain DESTINATION .)
install(FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/assets/capsicain.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/assets/AutoHotKey64.dll"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/platform/interception.dll"
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