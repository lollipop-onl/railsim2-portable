cmake_minimum_required(VERSION 3.24)

project(railsim2-portable LANGUAGES CXX)

# iconv for port/rs2_text CP932<->UTF-8 (#75). Not SDL; glibc is built-in on Linux.
find_package(Iconv REQUIRED)

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
  "${CMAKE_SOURCE_DIR}/port/path.cpp"
  "${CMAKE_SOURCE_DIR}/port/rs2_text.cpp"
  "${CMAKE_SOURCE_DIR}/port/ffp_state.cpp"
  "${CMAKE_SOURCE_DIR}/port/rs2_input.cpp"
  "${CMAKE_SOURCE_DIR}/lib/movie.cpp")

target_include_directories(railsim2_native SYSTEM BEFORE PRIVATE
  "${CMAKE_SOURCE_DIR}/port/stub"
)
target_include_directories(railsim2_native PRIVATE
  "${CMAKE_SOURCE_DIR}"
  "${CMAKE_SOURCE_DIR}/lib"
  "${CMAKE_SOURCE_DIR}/port"
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
target_link_libraries(railsim2_native PRIVATE Iconv::Iconv)

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

# Sample.rs2 Load/Save via CSaveFile (#36 / #54). Real Read/Save TUs for the
# layout object graph; port/rs2_roundtrip_stubs.cpp holds globals and UI seams.
set(RS2_ROUNDTRIP_SOURCES
  port/rs2_roundtrip.cpp
  port/rs2_roundtrip_stubs.cpp
  port/rs2_roundtrip_plugins.cpp
  port/rs2_roundtrip_seams.cpp
  port/rs2_roundtrip_ptr.cpp
  port/rs2_float_lexeme.cpp
  port/path.cpp
  port/rs2_text.cpp
  port/ffp_state.cpp
  CSaveFile.cpp
  Script.cpp
  md5.cpp
  Language.cpp
  WakeUp.cpp
  CPartsInst.cpp
  CModelSwitch.cpp
  CCustomizerMover.cpp
  CParticle.cpp
  CModelInst.cpp
  CRailConnector.cpp
  CRailCurve.cpp
  CRailDumpCurve.cpp
  CRailSplitCurve.cpp
  CRailDetectCurve.cpp
  CRailPlanCurve.cpp
  CLineBuildCurve.cpp
  CRailWay.cpp
  CTrainSetCurve.cpp
  CDiaInst.cpp
  CTrain.cpp
  CTrainGroup.cpp
  CCamera.cpp
  port/rs2_roundtrip_readmap.cpp
  CNamedObject.cpp
  CExpression.cpp
  CPier.cpp
  CLine.cpp
  CStation.cpp
  CStruct.cpp
  CScene.cpp
  CPlugin.cpp
)
add_executable(rs2_roundtrip ${RS2_ROUNDTRIP_SOURCES})
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
  RS2_ROUNDTRIP=1
  NOMINMAX
)
target_link_libraries(rs2_roundtrip PRIVATE Iconv::Iconv)

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
# Fixture missing -> 77 skip.
set_tests_properties(rs2_roundtrip PROPERTIES
  SKIP_RETURN_CODE 77
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

# Pointer ID 8-digit hex write / HexPointer-compatible parse (#40).
add_executable(rs2_ptr_test port/rs2_ptr_test.cpp)
target_include_directories(rs2_ptr_test PRIVATE "${CMAKE_SOURCE_DIR}/port")
target_compile_features(rs2_ptr_test PRIVATE cxx_std_17)
target_compile_options(rs2_ptr_test PRIVATE -Wall -Wextra)

add_test(NAME rs2_ptr_self_test COMMAND rs2_ptr_test --self-test)

# Save-side float %.6f write / ConstValue-compatible parse (#50).
add_executable(rs2_float_test port/rs2_float_test.cpp)
target_include_directories(rs2_float_test PRIVATE "${CMAKE_SOURCE_DIR}/port")
target_compile_features(rs2_float_test PRIVATE cxx_std_17)
target_compile_options(rs2_float_test PRIVATE -Wall -Wextra)

add_test(NAME rs2_float_self_test COMMAND rs2_float_test --self-test)

# MD5 / CheckLayoutDigest contract self-test (#44).
add_executable(rs2_md5_digest_test port/md5_digest_test.cpp md5.cpp)
target_include_directories(rs2_md5_digest_test PRIVATE "${CMAKE_SOURCE_DIR}")
target_compile_features(rs2_md5_digest_test PRIVATE cxx_std_17)
target_compile_options(rs2_md5_digest_test PRIVATE -Wall -Wextra -Wno-invalid-source-encoding)

add_test(NAME rs2_md5_self_test COMMAND rs2_md5_digest_test --self-test)

# CMesh::Load file-path probe via closed X-File parser (#45).
add_executable(rs2_cmesh_xfile_test port/rs2_cmesh_xfile_test.cpp port/rs2_cmesh_xfile.cpp port/xfile.cpp)
target_include_directories(rs2_cmesh_xfile_test SYSTEM BEFORE PRIVATE "${CMAKE_SOURCE_DIR}/port/stub")
target_include_directories(rs2_cmesh_xfile_test PRIVATE "${CMAKE_SOURCE_DIR}/port")
target_compile_features(rs2_cmesh_xfile_test PRIVATE cxx_std_17)
target_compile_options(rs2_cmesh_xfile_test PRIVATE -Wall -Wextra)
target_compile_definitions(rs2_cmesh_xfile_test PRIVATE RS2_PORTABLE_COMPILE_FIREWALL=1 NOMINMAX)

add_test(NAME rs2_cmesh_xfile_self_test COMMAND rs2_cmesh_xfile_test --self-test)
add_test(NAME rs2_cmesh_xfile_load COMMAND rs2_cmesh_xfile_test "${CMAKE_SOURCE_DIR}/Distribution")
set_tests_properties(rs2_cmesh_xfile_load PROPERTIES SKIP_RETURN_CODE 77)

# Config.txt smoke/roundtrip and *2.txt LoadHeader (#58). Does not link CConfigMode.
add_executable(rs2_config_plugin_test
  port/rs2_config_plugin_test.cpp
  port/rs2_config_plugin.cpp
  port/path.cpp)
target_include_directories(rs2_config_plugin_test PRIVATE "${CMAKE_SOURCE_DIR}/port")
target_compile_features(rs2_config_plugin_test PRIVATE cxx_std_17)
target_compile_options(rs2_config_plugin_test PRIVATE -Wall -Wextra)

add_test(NAME rs2_config_plugin_self_test COMMAND rs2_config_plugin_test --self-test)
add_test(NAME rs2_config_plugin_load COMMAND rs2_config_plugin_test
  "${CMAKE_SOURCE_DIR}/Distribution")
set_tests_properties(rs2_config_plugin_load PROPERTIES SKIP_RETURN_CODE 77)

# Closed FVF stride / CPU VB / DrawPrimitiveUP ring (#74). No GL draw.
add_executable(rs2_ffp_fvf_test port/ffp_fvf_test.cpp port/ffp_fvf.cpp port/ffp_state.cpp)
target_include_directories(rs2_ffp_fvf_test SYSTEM BEFORE PRIVATE
  "${CMAKE_SOURCE_DIR}/port/stub")
target_include_directories(rs2_ffp_fvf_test PRIVATE "${CMAKE_SOURCE_DIR}/port")
target_compile_features(rs2_ffp_fvf_test PRIVATE cxx_std_17)
target_compile_options(rs2_ffp_fvf_test PRIVATE -Wall -Wextra)
target_compile_definitions(rs2_ffp_fvf_test PRIVATE RS2_PORTABLE_COMPILE_FIREWALL=1 NOMINMAX)

add_test(NAME rs2_ffp_fvf_self_test COMMAND rs2_ffp_fvf_test --self-test)

# M3 core D3DRS / D3DTSS shadow + shader key (#80). No GLSL / GL.
add_executable(rs2_ffp_state_test
  port/ffp_state_test.cpp
  port/ffp_state.cpp
  port/ffp_fvf.cpp)
target_include_directories(rs2_ffp_state_test SYSTEM BEFORE PRIVATE
  "${CMAKE_SOURCE_DIR}/port/stub")
target_include_directories(rs2_ffp_state_test PRIVATE "${CMAKE_SOURCE_DIR}/port")
target_compile_features(rs2_ffp_state_test PRIVATE cxx_std_17)
target_compile_options(rs2_ffp_state_test PRIVATE -Wall -Wextra)
target_compile_definitions(rs2_ffp_state_test PRIVATE RS2_PORTABLE_COMPILE_FIREWALL=1 NOMINMAX)

add_test(NAME rs2_ffp_state_self_test COMMAND rs2_ffp_state_test --self-test)

# CP932 <-> UTF-8 and closed _mbsicmp replacement (#75). No SDL.
add_executable(rs2_text_test port/rs2_text_test.cpp port/rs2_text.cpp)
target_include_directories(rs2_text_test PRIVATE "${CMAKE_SOURCE_DIR}/port")
target_compile_features(rs2_text_test PRIVATE cxx_std_17)
target_compile_options(rs2_text_test PRIVATE -Wall -Wextra)
target_link_libraries(rs2_text_test PRIVATE Iconv::Iconv)

add_test(NAME rs2_text_self_test COMMAND rs2_text_test --self-test)

# PCM-only RIFF WAVE parser (#81). Does not swap CWave::Load or link OpenAL.
add_executable(rs2_wav_pcm_test port/wav_pcm_test.cpp port/wav_pcm.cpp)
target_include_directories(rs2_wav_pcm_test PRIVATE "${CMAKE_SOURCE_DIR}/port")
target_compile_features(rs2_wav_pcm_test PRIVATE cxx_std_17)
target_compile_options(rs2_wav_pcm_test PRIVATE -Wall -Wextra)

add_test(NAME rs2_wav_pcm_self_test COMMAND rs2_wav_pcm_test --self-test)
add_test(NAME rs2_wav_pcm_distribution COMMAND rs2_wav_pcm_test
  "${CMAKE_SOURCE_DIR}/Distribution")
set_tests_properties(rs2_wav_pcm_distribution PROPERTIES SKIP_RETURN_CODE 77)

# Stub keyboard/mouse/joy poll + ScanInputDevice merge (#82). No SDL.
add_executable(rs2_input_test port/rs2_input_test.cpp port/rs2_input.cpp)
target_include_directories(rs2_input_test SYSTEM BEFORE PRIVATE
  "${CMAKE_SOURCE_DIR}/port/stub")
target_include_directories(rs2_input_test PRIVATE "${CMAKE_SOURCE_DIR}/port")
target_compile_features(rs2_input_test PRIVATE cxx_std_17)
target_compile_options(rs2_input_test PRIVATE -Wall -Wextra)
target_compile_definitions(rs2_input_test PRIVATE RS2_PORTABLE_COMPILE_FIREWALL=1 NOMINMAX)

add_test(NAME rs2_input_self_test COMMAND rs2_input_test --self-test)
