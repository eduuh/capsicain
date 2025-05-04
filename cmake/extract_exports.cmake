# CMake script to process exports from interception.dll
# This script replaces the need for the batch script approach

# Verify required variables are set
if(NOT DEFINED EXPORTS_FILE)
  message(FATAL_ERROR "EXPORTS_FILE not defined")
endif()

if(NOT DEFINED OUTPUT_DEF)
  message(FATAL_ERROR "OUTPUT_DEF not defined")
endif()

message(STATUS "Processing exports from ${EXPORTS_FILE}")
message(STATUS "Creating def file at ${OUTPUT_DEF}")

# Read the exports file
file(READ "${EXPORTS_FILE}" EXPORTS_CONTENT)

# Write the header of the DEF file
file(WRITE "${OUTPUT_DEF}" "LIBRARY interception\nEXPORTS\n")

# Process the content line by line to extract function names
# This mimics what the batch script was doing with findstr and for loop
string(REGEX REPLACE "\r?\n" ";" EXPORTS_LINES "${EXPORTS_CONTENT}")

foreach(LINE ${EXPORTS_LINES})
  # Match lines containing exported symbols (format typical of dumpbin output)
  if(LINE MATCHES "^\\s*[0-9]+\\s+[0-9A-F]+\\s+[0-9A-F]+\\s+([A-Za-z0-9_@]+).*$")
    set(FUNCTION_NAME ${CMAKE_MATCH_1})
    file(APPEND "${OUTPUT_DEF}" "${FUNCTION_NAME}\n")
    message(STATUS "  Found export: ${FUNCTION_NAME}")
  endif()
endforeach()

message(STATUS "Export definition file created at ${OUTPUT_DEF}")