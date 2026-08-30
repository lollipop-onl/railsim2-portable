cmake_minimum_required(VERSION 3.24)

project(railsim2-portable LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(RS2_NATIVE_TOTAL 254 CACHE STRING "Root-level game sources tracked for compile progress")

function(rs2_read_native_sources out_var)
  file(READ "${CMAKE_SOURCE_DIR}/port/native_sources.txt" _raw)
  string(REPLACE "\r\n" "\n" _raw "${_raw}")
  string(REPLACE "\r" "\n" _raw "${_raw}")
  string(REPLACE "\n" ";" _lines "${_raw}")
  set(_sources "")
  foreach(_line IN LISTS _lines)
    string(STRIP "${_line}" _line)
    if(_line STREQUAL "" OR _line MATCHES "^#")
      continue()
    endif()
    list(APPEND _sources "${CMAKE_SOURCE_DIR}/${_line}")
  endforeach()
  set(${out_var} "${_sources}" PARENT_SCOPE)
endfunction()

rs2_read_native_sources(RS2_NATIVE_SOURCES)

if(RS2_NATIVE_SOURCES STREQUAL "")
  message(FATAL_ERROR "port/native_sources.txt has no compile targets")
endif()

add_library(railsim2_native STATIC ${RS2_NATIVE_SOURCES})

target_include_directories(railsim2_native SYSTEM BEFORE PRIVATE
  "${CMAKE_SOURCE_DIR}/port/stub"
)
target_include_directories(railsim2_native PRIVATE
  "${CMAKE_SOURCE_DIR}"
  "${CMAKE_SOURCE_DIR}/lib"
)

target_compile_options(railsim2_native PRIVATE
  -Wall
  -Wextra
  -Wno-invalid-source-encoding
  -Wno-address-of-temporary
)

target_compile_definitions(railsim2_native PRIVATE
  RS2_PORTABLE_COMPILE_FIREWALL=1
  NOMINMAX
)

# Link skeleton: POSIX main only. Allowlisted game objects compile into
# railsim2_native.a but are not linked yet — they still need udx globals
# (sv3, g_frame, …) from lib/. SDL2 is not a check-preset dependency.
# Win32 .rc is skipped; icons stay as files until a later loader.
add_executable(railsim2 port/native_entry.cpp)
add_dependencies(railsim2 railsim2_native)
target_compile_definitions(railsim2 PRIVATE
  RS2_PORTABLE_COMPILE_FIREWALL=1
  NOMINMAX
)

enable_testing()
add_test(NAME rs2_smoke COMMAND "${CMAKE_COMMAND}" -E echo "ctest skeleton ok")
