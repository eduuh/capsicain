# Tests CMakeLists.txt
# Unit tests for capsicain using Google Test

# Define test source files
set(TEST_SOURCES
    test_main.cpp
    test_utils.cpp
    test_modifiers.cpp
    test_scancodes.cpp
    test_tap_detector.cpp
    test_modifier_tracker.cpp
    test_combo_matcher.cpp
    test_key_mapper.cpp
    test_key_processor.cpp
)

# Define source files to test (excluding main app entry point and Windows-specific code)
set(CAPSICAIN_TEST_SOURCES
    ${CMAKE_SOURCE_DIR}/src/legacy/utils.cpp
    ${CMAKE_SOURCE_DIR}/src/legacy/modifiers.cpp
    ${CMAKE_SOURCE_DIR}/src/legacy/scancodes.cpp
)

# Create test executable
add_executable(capsicain_tests
    ${TEST_SOURCES}
    ${CAPSICAIN_TEST_SOURCES}
)

# Include directories for tests - src is root
target_include_directories(capsicain_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/domain
)

# Link Google Test
target_link_libraries(capsicain_tests
    GTest::gtest
    GTest::gtest_main
)

# Windows-specific settings for tests
if(WIN32)
    target_compile_definitions(capsicain_tests PRIVATE
        _CRT_SECURE_NO_WARNINGS
        NOMINMAX
    )
    target_link_libraries(capsicain_tests
        user32
        advapi32
        shell32
        ole32
    )
endif()

# Register tests with CTest
include(GoogleTest)
gtest_discover_tests(capsicain_tests)
