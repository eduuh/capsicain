cmake_minimum_required(VERSION 3.15)
project(capsicain VERSION 0.97.0 LANGUAGES CXX)

# Set C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Turn off static linking of interception.dll - use dynamic linking instead
option(STATIC_LINK_INTERCEPTION "Statically link interception library" OFF)

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

# Static linking of runtime libraries
if(MSVC)
    # Static linking for MSVC
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    foreach(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
        if(${flag_var} MATCHES "/MD")
            string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
        endif()
    endforeach()
endif()

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

# Resource files handling based on compiler
if(MSVC)
    set(CAPSICAIN_RESOURCES
        capsicain/capsicain.rc
    )
elseif(MINGW OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND WIN32))
    # For MinGW or Clang on Windows, compile the .rc file separately
    set(RC_FILE "${CMAKE_CURRENT_SOURCE_DIR}/capsicain/capsicain.rc")
    set(RES_FILE "${CMAKE_CURRENT_BINARY_DIR}/capsicain_res.o")
    
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND WIN32)
        # Use llvm-rc for Clang on Windows if available
        find_program(LLVM_RC llvm-rc)
        if(LLVM_RC)
            add_custom_command(
                OUTPUT ${RES_FILE}
                COMMAND ${LLVM_RC} -I${CMAKE_CURRENT_SOURCE_DIR}/capsicain
                           -i${RC_FILE}
                           -o${RES_FILE}
                           -DNDEBUG
                DEPENDS ${RC_FILE}
                COMMENT "Compiling resources with llvm-rc"
            )
        else()
            # Fall back to windres if llvm-rc is not found
            add_custom_command(
                OUTPUT ${RES_FILE}
                COMMAND windres -I${CMAKE_CURRENT_SOURCE_DIR}/capsicain
                           -i${RC_FILE}
                           -o${RES_FILE}
                           -DNDEBUG
                DEPENDS ${RC_FILE}
                COMMENT "Compiling resources with windres"
            )
        endif()
    else()
        # Use windres for MinGW
        add_custom_command(
            OUTPUT ${RES_FILE}
            COMMAND windres -I${CMAKE_CURRENT_SOURCE_DIR}/capsicain
                       -i${RC_FILE}
                       -o${RES_FILE}
                       -DNDEBUG
            DEPENDS ${RC_FILE}
            COMMENT "Compiling resources with windres"
        )
    endif()
    
    # Include the compiled resource
    list(APPEND CAPSICAIN_SOURCES ${RES_FILE})
    set(CAPSICAIN_RESOURCES "")
endif()

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
message(STATUS "Static linking interception: ${STATIC_LINK_INTERCEPTION}")

# Precompiled headers
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.16" AND 
   (NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "8.0"))
    target_precompile_headers(capsicain PRIVATE capsicain/pch.h)
    set_source_files_properties(capsicain/utils.cpp PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
endif()

# Compiler-specific settings
if(MSVC)
    # MSVC specific flags
    target_compile_options(capsicain PRIVATE /W3)
    target_compile_definitions(capsicain PRIVATE _CRT_SECURE_NO_WARNINGS)
    
    # Force static linking
    target_link_options(capsicain PRIVATE /INCREMENTAL:NO)
    if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_link_options(capsicain PRIVATE /LTCG)
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # Clang specific flags
    target_compile_options(capsicain PRIVATE 
        -Wall 
        -Wextra
        -Wno-unused-parameter
        -Wno-missing-field-initializers
    )
    
    if(WIN32)
        # Clang on Windows
        target_compile_definitions(capsicain PRIVATE WIN32)
        
        # GUI application on Windows
        if(NOT WINDOWS_STORE)
            target_link_options(capsicain PRIVATE -mwindows)
        endif()
        
        # Static linking when using Clang on Windows
        if(NOT MINGW)
            target_link_options(capsicain PRIVATE -static)
        endif()
    endif()
else()
    # GCC/MinGW specific flags
    target_compile_options(capsicain PRIVATE 
        -Wall 
        -Wextra
        -Wno-unused-parameter
        -Wno-missing-field-initializers
    )
    
    # Static linking for MinGW
    if(MINGW)
        target_link_options(capsicain PRIVATE -static -static-libgcc -static-libstdc++)
        target_compile_definitions(capsicain PRIVATE WIN32)
        target_link_options(capsicain PRIVATE -mwindows)
    endif()
endif()

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

# Dynamic linking of interception.dll (since STATIC_LINK_INTERCEPTION is OFF)
# Set up path to interception.lib
if(MSVC)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/capsicain/interception.lib")
        set(INTERCEPTION_LIB "${CMAKE_CURRENT_SOURCE_DIR}/capsicain/interception.lib")
    elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/capsicain/interception/interception.lib")
        set(INTERCEPTION_LIB "${CMAKE_CURRENT_SOURCE_DIR}/capsicain/interception/interception.lib")
    else()
        message(FATAL_ERROR "Could not find interception.lib")
    endif()
    target_link_libraries(capsicain ${INTERCEPTION_LIB})
elseif(MINGW OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND WIN32 AND NOT MSVC))
    # For MinGW or Clang on Windows, generate import library from DLL
    set(INTERCEPTION_DLL "${CMAKE_CURRENT_SOURCE_DIR}/capsicain/interception.dll")
    set(INTERCEPTION_DEF "${CMAKE_CURRENT_BINARY_DIR}/interception.def")
    set(INTERCEPTION_IMPORT_LIB "${CMAKE_CURRENT_BINARY_DIR}/libinterception.a")
    
    # Create import library
    add_custom_command(TARGET capsicain PRE_LINK
        COMMAND ${CMAKE_COMMAND} -E echo "Creating import library for MinGW/Clang"
        COMMAND gendef - "${INTERCEPTION_DLL}" > "${INTERCEPTION_DEF}"
        COMMAND dlltool --input-def "${INTERCEPTION_DEF}" --output-lib "${INTERCEPTION_IMPORT_LIB}"
        COMMENT "Converting interception.dll to import library"
    )
    
    target_link_libraries(capsicain ${INTERCEPTION_IMPORT_LIB})
endif()

# Always copy the interception.dll to the output directory
add_custom_command(TARGET capsicain POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/capsicain/interception.dll"
        $<TARGET_FILE_DIR:capsicain>
    COMMENT "Copying interception.dll to output directory"
)

# Update installation rules to include interception.dll
install(TARGETS capsicain DESTINATION .)
install(FILES 
    "${CMAKE_CURRENT_SOURCE_DIR}/capsicain/capsicain.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/capsicain/interception.dll"
    "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
    DESTINATION .
)