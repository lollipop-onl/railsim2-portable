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

add_library(railsim2_native STATIC ${RS2_NATIVE_SOURCES}
  "${CMAKE_SOURCE_DIR}/port/path.cpp")

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

# Sample.rs2 Load/Save via CSaveFile (#36). Stubs fill the object graph;
# byte-identity remains parent #10. Does not link the full railsim2_native.a.
add_executable(rs2_roundtrip
  port/rs2_roundtrip.cpp
  port/rs2_roundtrip_stubs.cpp
  port/path.cpp
  CSaveFile.cpp
  Script.cpp
  md5.cpp
)
target_include_directories(rs2_roundtrip SYSTEM BEFORE PRIVATE
  "${CMAKE_SOURCE_DIR}/port/stub"
)
target_include_directories(rs2_roundtrip PRIVATE
  "${CMAKE_SOURCE_DIR}"
  "${CMAKE_SOURCE_DIR}/lib"
  "${CMAKE_SOURCE_DIR}/port"
)
target_compile_features(rs2_roundtrip PRIVATE cxx_std_17)
target_compile_options(rs2_roundtrip PRIVATE
  -Wall
  -Wextra
  -Wno-invalid-source-encoding
  -Wno-address-of-temporary
)
target_compile_definitions(rs2_roundtrip PRIVATE
  RS2_PORTABLE_COMPILE_FIREWALL=1
  NOMINMAX
)

set(RS2_ROUNDTRIP_FIXTURE
  "${CMAKE_SOURCE_DIR}/Distribution/en/RailSim2/Layout/Sample.rs2")
# Save requires a basename under Layout (CheckSlash); compare uses this path.
set(RS2_ROUNDTRIP_OUTPUT
  "${CMAKE_SOURCE_DIR}/Distribution/en/RailSim2/Layout/rs2_roundtrip_out.rs2")

enable_testing()
add_test(NAME rs2_smoke COMMAND "${CMAKE_COMMAND}" -E echo "ctest skeleton ok")
add_test(NAME rs2_roundtrip_reports_diff COMMAND rs2_roundtrip --self-test-diff)
add_test(NAME rs2_roundtrip COMMAND rs2_roundtrip
  "${RS2_ROUNDTRIP_FIXTURE}"
  "${RS2_ROUNDTRIP_OUTPUT}")
# Fixture missing -> 77 skip. Expected Load/Save mismatch -> 1 (WILL_FAIL).
set_tests_properties(rs2_roundtrip PROPERTIES
  SKIP_RETURN_CODE 77
  WILL_FAIL TRUE
)

# Closed text X-File parser (#28). Does not replace CMesh::Load (#6).
add_executable(rs2_xfile_load port/xfile_load.cpp port/xfile.cpp)
target_include_directories(rs2_xfile_load PRIVATE "${CMAKE_SOURCE_DIR}/port")
target_compile_features(rs2_xfile_load PRIVATE cxx_std_17)
target_compile_options(rs2_xfile_load PRIVATE -Wall -Wextra)

add_test(NAME rs2_xfile_self_test COMMAND rs2_xfile_load --self-test)
add_test(NAME rs2_xfile_load COMMAND rs2_xfile_load "${CMAKE_SOURCE_DIR}/Distribution")
set_tests_properties(rs2_xfile_load PROPERTIES SKIP_RETURN_CODE 77)

# Closed-set path join / list / fullpath (#32). Does not rewrite plugin Load().
add_executable(rs2_path_test port/path_test.cpp port/path.cpp)
target_include_directories(rs2_path_test PRIVATE "${CMAKE_SOURCE_DIR}/port")
target_compile_features(rs2_path_test PRIVATE cxx_std_17)
target_compile_options(rs2_path_test PRIVATE -Wall -Wextra)

add_test(NAME rs2_path_self_test COMMAND rs2_path_test --self-test)
add_test(NAME rs2_path_distribution COMMAND rs2_path_test "${CMAKE_SOURCE_DIR}/Distribution")
set_tests_properties(rs2_path_distribution PROPERTIES SKIP_RETURN_CODE 77)
