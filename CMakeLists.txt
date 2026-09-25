cmake_minimum_required(VERSION 3.24)

project(railsim2-portable LANGUAGES CXX)

# iconv for port/rs2_text CP932<->UTF-8 (#75). Not SDL; glibc is built-in on Linux.
find_package(Iconv REQUIRED)

# Optional runtime deps (#105). The check preset leaves RS2_RUNTIME OFF and
# must not search SDL2 / OpenGL / OpenAL. CI apt does not install them.
# find_package is QUIET (not REQUIRED): missing packages leave the feature off.
option(RS2_RUNTIME "Search optional SDL2 / OpenGL / OpenAL (runtime preset)" OFF)
set(RS2_HAVE_SDL2 OFF)
set(RS2_HAVE_OPENGL OFF)
set(RS2_HAVE_OPENAL OFF)
if(RS2_RUNTIME)
  find_package(SDL2 CONFIG QUIET)
  if(NOT SDL2_FOUND)
    find_package(SDL2 QUIET)
  endif()
  if(SDL2_FOUND OR TARGET SDL2::SDL2)
    set(RS2_HAVE_SDL2 ON)
  endif()

  find_package(OpenGL QUIET)
  if(OpenGL_FOUND OR OPENGL_FOUND)
    set(RS2_HAVE_OPENGL ON)
  endif()

  find_package(OpenAL QUIET)
  if(OpenAL_FOUND OR OPENAL_FOUND)
    set(RS2_HAVE_OPENAL ON)
  endif()

  message(STATUS "RS2 runtime SDL2: ${RS2_HAVE_SDL2}")
  message(STATUS "RS2 runtime OpenGL: ${RS2_HAVE_OPENGL}")
  message(STATUS "RS2 runtime OpenAL: ${RS2_HAVE_OPENAL}")
else()
  message(STATUS "RS2 runtime: off (check preset; SDL2/GL/OpenAL not searched)")
endif()
set(RS2_HAVE_SDL2 "${RS2_HAVE_SDL2}" CACHE BOOL "SDL2 found (runtime preset; unused by check)" FORCE)
set(RS2_HAVE_OPENGL "${RS2_HAVE_OPENGL}" CACHE BOOL "OpenGL found (runtime preset; unused by check)" FORCE)
set(RS2_HAVE_OPENAL "${RS2_HAVE_OPENAL}" CACHE BOOL "OpenAL found (runtime preset; unused by check)" FORCE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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

# Only port/ backing code belongs in this literal list. A game source named
# here as well as in port/native_sources.txt still builds -- CMake keeps the
# duplicate in the SOURCES property and de-duplicates only in the generator,
# by object name, with no warning -- but scripts/progress.sh counts the
# allowlist alone, so naming one here instead leaves it out of the numerator.
add_library(railsim2_native STATIC ${RS2_NATIVE_SOURCES}
  "${CMAKE_SOURCE_DIR}/port/path.cpp"
  "${CMAKE_SOURCE_DIR}/port/d3dx_math.cpp"
  "${CMAKE_SOURCE_DIR}/port/rs2_text.cpp"
  "${CMAKE_SOURCE_DIR}/port/xfile.cpp"
  "${CMAKE_SOURCE_DIR}/port/rs2_cmesh_xfile.cpp"
  "${CMAKE_SOURCE_DIR}/port/ffp_state.cpp"
  "${CMAKE_SOURCE_DIR}/port/ffp_fvf.cpp"
  "${CMAKE_SOURCE_DIR}/port/ffp_glsl.cpp"
  "${CMAKE_SOURCE_DIR}/port/ffp_program.cpp"
  "${CMAKE_SOURCE_DIR}/port/ffp_gl.cpp"
  "${CMAKE_SOURCE_DIR}/port/ffp_window.cpp"
  "${CMAKE_SOURCE_DIR}/port/rs2_input.cpp"
  "${CMAKE_SOURCE_DIR}/port/rs2_input_sdl.cpp"
  "${CMAKE_SOURCE_DIR}/port/rs2_msg.cpp"
  "${CMAKE_SOURCE_DIR}/port/rs2_msg_sdl.cpp"
  "${CMAKE_SOURCE_DIR}/port/wav_pcm.cpp"
  "${CMAKE_SOURCE_DIR}/port/rs2_audio.cpp"
  "${CMAKE_SOURCE_DIR}/port/rs2_ime.cpp"
  "${CMAKE_SOURCE_DIR}/port/rs2_media_globals.cpp")

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
  -Wnonportable-include-path
  -Werror=nonportable-include-path
)

target_compile_definitions(railsim2_native PRIVATE
  RS2_PORTABLE_COMPILE_FIREWALL=1
  NOMINMAX
  RS2_HAVE_OPENGL=$<BOOL:${RS2_HAVE_OPENGL}>
  RS2_HAVE_SDL2=$<BOOL:${RS2_HAVE_SDL2}>
)
target_link_libraries(railsim2_native PRIVATE Iconv::Iconv)
if(RS2_HAVE_OPENGL)
  target_link_libraries(railsim2_native PRIVATE OpenGL::GL)
endif()
if(RS2_HAVE_SDL2)
  target_link_libraries(railsim2_native PRIVATE SDL2::SDL2)
endif()

# Whole archive, not a plain link: nothing reachable from WinMain references
# some game TUs (lib/music.cpp and lib/movie.cpp, for example), so the
# linker would leave them out (docs/porting/link-seams.md).
# Win32 .rc is skipped; icons stay as files until a later loader.
add_executable(railsim2 port/native_entry.cpp)
target_include_directories(railsim2 SYSTEM BEFORE PRIVATE
  "${CMAKE_SOURCE_DIR}/port/stub"
)
target_compile_options(railsim2 PRIVATE -Wall -Wextra)
target_compile_definitions(railsim2 PRIVATE
  RS2_PORTABLE_COMPILE_FIREWALL=1
  NOMINMAX
)
target_link_libraries(railsim2 PRIVATE "$<LINK_LIBRARY:WHOLE_ARCHIVE,railsim2_native>")

# Sample.rs2 Load/Save via CSaveFile (#36 / #54). Real Read/Save TUs for the
# layout object graph; port/rs2_roundtrip_stubs.cpp holds globals and UI seams.
set(RS2_ROUNDTRIP_SOURCES
  port/rs2_roundtrip.cpp
  port/rs2_roundtrip_stubs.cpp
  port/rs2_roundtrip_plugins.cpp
  port/rs2_roundtrip_seams.cpp
  port/rs2_roundtrip_ptr.cpp
  port/rs2_roundtrip_color.cpp
  port/rs2_float_lexeme.cpp
  port/path.cpp
  port/rs2_text.cpp
  port/ffp_state.cpp
  port/d3dx_math.cpp
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
# AsgnColor / ScaleColor at Win32's 32-bit D3DCOLOR (#203).
add_test(NAME rs2_plugin_color_self_test COMMAND rs2_roundtrip --self-test-color)
add_test(NAME rs2_roundtrip COMMAND rs2_roundtrip
  "${RS2_ROUNDTRIP_FIXTURE}"
  "${RS2_ROUNDTRIP_OUTPUT}")
# Fixture missing -> 77 skip.
set_tests_properties(rs2_roundtrip PROPERTIES
  SKIP_RETURN_CODE 77
)

add_test(NAME rs2_headless_start
  COMMAND "${CMAKE_COMMAND}" "-DRAILSIM2=$<TARGET_FILE:railsim2>"
    -P "${CMAKE_SOURCE_DIR}/port/railsim2_headless_test.cmake"
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
add_test(NAME rs2_whole_archive_members
  COMMAND "${CMAKE_COMMAND}" "-DRAILSIM2=$<TARGET_FILE:railsim2>" "-DNM=${CMAKE_NM}"
    -P "${CMAKE_SOURCE_DIR}/port/railsim2_whole_archive_test.cmake")

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
add_executable(rs2_ffp_fvf_test
  port/ffp_fvf_test.cpp
  port/ffp_fvf.cpp
  port/ffp_state.cpp
  port/ffp_glsl.cpp
  port/ffp_program.cpp)
target_include_directories(rs2_ffp_fvf_test SYSTEM BEFORE PRIVATE
  "${CMAKE_SOURCE_DIR}/port/stub")
target_include_directories(rs2_ffp_fvf_test PRIVATE "${CMAKE_SOURCE_DIR}/port")
target_compile_features(rs2_ffp_fvf_test PRIVATE cxx_std_17)
target_compile_options(rs2_ffp_fvf_test PRIVATE -Wall -Wextra)
target_compile_definitions(rs2_ffp_fvf_test PRIVATE RS2_PORTABLE_COMPILE_FIREWALL=1 NOMINMAX)

add_test(NAME rs2_ffp_fvf_self_test COMMAND rs2_ffp_fvf_test --self-test)

# lib/vertex.h VTX_* sizeof / offsetof against the closed FVF table (#203).
add_executable(rs2_vertex_layout_test
  port/vertex_layout_test.cpp
  port/ffp_fvf.cpp
  port/ffp_state.cpp
  port/ffp_glsl.cpp
  port/ffp_program.cpp)
target_include_directories(rs2_vertex_layout_test SYSTEM BEFORE PRIVATE
  "${CMAKE_SOURCE_DIR}/port/stub")
target_include_directories(rs2_vertex_layout_test PRIVATE
  "${CMAKE_SOURCE_DIR}/lib"
  "${CMAKE_SOURCE_DIR}/port")
target_compile_features(rs2_vertex_layout_test PRIVATE cxx_std_17)
target_compile_options(rs2_vertex_layout_test PRIVATE
  -Wall
  -Wextra
  -Wno-invalid-source-encoding)
target_compile_definitions(rs2_vertex_layout_test PRIVATE RS2_PORTABLE_COMPILE_FIREWALL=1 NOMINMAX)

add_test(NAME rs2_vertex_layout_self_test COMMAND rs2_vertex_layout_test --self-test)

# M3 core D3DRS / D3DTSS shadow + shader key (#80). No GLSL / GL.
add_executable(rs2_ffp_state_test
  port/ffp_state_test.cpp
  port/ffp_state.cpp
  port/ffp_fvf.cpp
  port/ffp_glsl.cpp
  port/ffp_program.cpp)
target_include_directories(rs2_ffp_state_test SYSTEM BEFORE PRIVATE
  "${CMAKE_SOURCE_DIR}/port/stub")
target_include_directories(rs2_ffp_state_test PRIVATE "${CMAKE_SOURCE_DIR}/port")
target_compile_features(rs2_ffp_state_test PRIVATE cxx_std_17)
target_compile_options(rs2_ffp_state_test PRIVATE -Wall -Wextra)
target_compile_definitions(rs2_ffp_state_test PRIVATE RS2_PORTABLE_COMPILE_FIREWALL=1 NOMINMAX)

add_test(NAME rs2_ffp_state_self_test COMMAND rs2_ffp_state_test --self-test)

# M3 core FVF x state key -> GLSL 330 core (#88). No GL context / draw.
add_executable(rs2_ffp_glsl_test
  port/ffp_glsl_test.cpp
  port/ffp_glsl.cpp
  port/ffp_state.cpp
  port/ffp_fvf.cpp
  port/ffp_program.cpp)
target_include_directories(rs2_ffp_glsl_test SYSTEM BEFORE PRIVATE
  "${CMAKE_SOURCE_DIR}/port/stub")
target_include_directories(rs2_ffp_glsl_test PRIVATE "${CMAKE_SOURCE_DIR}/port")
target_compile_features(rs2_ffp_glsl_test PRIVATE cxx_std_17)
target_compile_options(rs2_ffp_glsl_test PRIVATE -Wall -Wextra)
target_compile_definitions(rs2_ffp_glsl_test PRIVATE RS2_PORTABLE_COMPILE_FIREWALL=1 NOMINMAX)

add_test(NAME rs2_ffp_glsl_self_test COMMAND rs2_ffp_glsl_test --self-test)

# Interned FFP program handle + stub DrawPrimitiveUP (#92). Device UP also
# tries GL after the CPU record (#120); this TU links ffp_gl.cpp compiled
# with RS2_HAVE_OPENGL=0 so the try is a no-op.
add_executable(rs2_ffp_program_test
  port/ffp_program_test.cpp
  port/ffp_program.cpp
  port/ffp_glsl.cpp
  port/ffp_state.cpp
  port/ffp_fvf.cpp
  port/ffp_gl.cpp)
target_include_directories(rs2_ffp_program_test SYSTEM BEFORE PRIVATE
  "${CMAKE_SOURCE_DIR}/port/stub")
target_include_directories(rs2_ffp_program_test PRIVATE "${CMAKE_SOURCE_DIR}/port")
target_compile_features(rs2_ffp_program_test PRIVATE cxx_std_17)
target_compile_options(rs2_ffp_program_test PRIVATE -Wall -Wextra)
target_compile_definitions(rs2_ffp_program_test PRIVATE
  RS2_PORTABLE_COMPILE_FIREWALL=1
  NOMINMAX
  RS2_HAVE_OPENGL=0)

add_test(NAME rs2_ffp_program_self_test COMMAND rs2_ffp_program_test --self-test)

# Link interned FFP GLSL (#111), draw UP records through a VBO (#114),
# and apply FFP uniforms / CPU RGBA bind (#118).
# check/CI: RS2_HAVE_OPENGL off, no SDL window. The test TU always compiles
# with RS2_HAVE_OPENGL=0 so ctest verifies no-GL link/draw/apply/bind
# failure even when the runtime preset found OpenGL for railsim2_native.
add_executable(rs2_ffp_gl_test
  port/ffp_gl_test.cpp
  port/ffp_gl.cpp
  port/ffp_program.cpp
  port/ffp_glsl.cpp
  port/ffp_state.cpp
  port/ffp_fvf.cpp)
target_include_directories(rs2_ffp_gl_test SYSTEM BEFORE PRIVATE
  "${CMAKE_SOURCE_DIR}/port/stub")
target_include_directories(rs2_ffp_gl_test PRIVATE "${CMAKE_SOURCE_DIR}/port")
target_compile_features(rs2_ffp_gl_test PRIVATE cxx_std_17)
target_compile_options(rs2_ffp_gl_test PRIVATE -Wall -Wextra)
target_compile_definitions(rs2_ffp_gl_test PRIVATE
  RS2_PORTABLE_COMPILE_FIREWALL=1
  NOMINMAX
  RS2_HAVE_OPENGL=0)

add_test(NAME rs2_ffp_gl_self_test COMMAND rs2_ffp_gl_test --self-test)

# SDL window + GL 3.3 core context (#116). check/CI: both feature flags
# off, no SDL window. The test TU always compiles with RS2_HAVE_SDL2=0
# and RS2_HAVE_OPENGL=0 so ctest verifies create/present/destroy failure
# even when the runtime preset found SDL2/GL for railsim2_native.
add_executable(rs2_ffp_window_test
  port/ffp_window_test.cpp
  port/ffp_window.cpp)
target_include_directories(rs2_ffp_window_test PRIVATE "${CMAKE_SOURCE_DIR}/port")
target_compile_features(rs2_ffp_window_test PRIVATE cxx_std_17)
target_compile_options(rs2_ffp_window_test PRIVATE -Wall -Wextra)
target_compile_definitions(rs2_ffp_window_test PRIVATE
  RS2_PORTABLE_COMPILE_FIREWALL=1
  NOMINMAX
  RS2_HAVE_SDL2=0
  RS2_HAVE_OPENGL=0)

add_test(NAME rs2_ffp_window_self_test COMMAND rs2_ffp_window_test --self-test)

# Wire DrawPrimitiveUP / Present / Clear / SetVertexShader (#120).
# check/CI: OpenGL and SDL2 off, no SDL window. Present without a created
# window is S_OK; DrawPrimitiveUP still records on the CPU.
add_executable(rs2_ffp_device_test
  port/ffp_device_test.cpp
  port/ffp_gl.cpp
  port/ffp_window.cpp
  port/ffp_program.cpp
  port/ffp_glsl.cpp
  port/ffp_state.cpp
  port/ffp_fvf.cpp)
target_include_directories(rs2_ffp_device_test SYSTEM BEFORE PRIVATE
  "${CMAKE_SOURCE_DIR}/port/stub")
target_include_directories(rs2_ffp_device_test PRIVATE "${CMAKE_SOURCE_DIR}/port")
target_compile_features(rs2_ffp_device_test PRIVATE cxx_std_17)
target_compile_options(rs2_ffp_device_test PRIVATE -Wall -Wextra)
target_compile_definitions(rs2_ffp_device_test PRIVATE
  RS2_PORTABLE_COMPILE_FIREWALL=1
  NOMINMAX
  RS2_HAVE_OPENGL=0
  RS2_HAVE_SDL2=0)

add_test(NAME rs2_ffp_device_self_test COMMAND rs2_ffp_device_test --self-test)

# Runtime-only smoke: solid UP triangle through create/draw/present/destroy.
# Not a ctest: it needs a display. The runtime workflow runs it on Linux
# under Xvfb. Missing SDL2 or OpenGL leaves the target out.
if(RS2_RUNTIME AND RS2_HAVE_SDL2 AND RS2_HAVE_OPENGL)
  add_executable(rs2_ffp_smoke
    port/ffp_smoke.cpp
    port/ffp_gl.cpp
    port/ffp_window.cpp
    port/ffp_program.cpp
    port/ffp_glsl.cpp
    port/ffp_state.cpp
    port/ffp_fvf.cpp)
  target_include_directories(rs2_ffp_smoke SYSTEM BEFORE PRIVATE
    "${CMAKE_SOURCE_DIR}/port/stub")
  target_include_directories(rs2_ffp_smoke PRIVATE "${CMAKE_SOURCE_DIR}/port")
  target_compile_features(rs2_ffp_smoke PRIVATE cxx_std_17)
  target_compile_options(rs2_ffp_smoke PRIVATE -Wall -Wextra)
  target_compile_definitions(rs2_ffp_smoke PRIVATE
    RS2_PORTABLE_COMPILE_FIREWALL=1
    NOMINMAX
    RS2_HAVE_OPENGL=1
    RS2_HAVE_SDL2=1)
  target_link_libraries(rs2_ffp_smoke PRIVATE OpenGL::GL SDL2::SDL2)
endif()

# Runtime-only: the SDL input poll and the inactive message wait discard the
# input events nothing reads, so SDL_QUIT still queues after a flood (#212).
# Not a ctest: it needs a video driver. The runtime workflow runs it on Linux
# under Xvfb.
if(RS2_RUNTIME AND RS2_HAVE_SDL2)
  add_executable(rs2_input_sdl_test
    port/rs2_input_sdl_test.cpp
    port/rs2_input_sdl.cpp
    port/rs2_msg.cpp
    port/rs2_msg_sdl.cpp
    port/ffp_window.cpp)
  target_include_directories(rs2_input_sdl_test SYSTEM BEFORE PRIVATE
    "${CMAKE_SOURCE_DIR}/port/stub")
  target_include_directories(rs2_input_sdl_test PRIVATE "${CMAKE_SOURCE_DIR}/port")
  target_compile_features(rs2_input_sdl_test PRIVATE cxx_std_17)
  target_compile_options(rs2_input_sdl_test PRIVATE -Wall -Wextra)
  target_compile_definitions(rs2_input_sdl_test PRIVATE
    RS2_PORTABLE_COMPILE_FIREWALL=1
    NOMINMAX
    RS2_HAVE_OPENGL=0
    RS2_HAVE_SDL2=1)
  target_link_libraries(rs2_input_sdl_test PRIVATE SDL2::SDL2)
endif()

# CP932 <-> UTF-8 and closed _mbsicmp replacement (#75). No SDL.
add_executable(rs2_text_test port/rs2_text_test.cpp port/rs2_text.cpp)
target_include_directories(rs2_text_test PRIVATE "${CMAKE_SOURCE_DIR}/port")
target_compile_features(rs2_text_test PRIVATE cxx_std_17)
target_compile_options(rs2_text_test PRIVATE -Wall -Wextra)
target_link_libraries(rs2_text_test PRIVATE Iconv::Iconv)

add_test(NAME rs2_text_self_test COMMAND rs2_text_test --self-test)

# PCM-only RIFF WAVE parser (#81). CWave::Load calls it (#86). No OpenAL.
add_executable(rs2_wav_pcm_test port/wav_pcm_test.cpp port/wav_pcm.cpp)
target_include_directories(rs2_wav_pcm_test PRIVATE "${CMAKE_SOURCE_DIR}/port")
target_compile_features(rs2_wav_pcm_test PRIVATE cxx_std_17)
target_compile_options(rs2_wav_pcm_test PRIVATE -Wall -Wextra)

add_test(NAME rs2_wav_pcm_self_test COMMAND rs2_wav_pcm_test --self-test)
add_test(NAME rs2_wav_pcm_distribution COMMAND rs2_wav_pcm_test
  "${CMAKE_SOURCE_DIR}/Distribution")
set_tests_properties(rs2_wav_pcm_distribution PROPERTIES SKIP_RETURN_CODE 77)

# CWave::Load maps Rs2WavPcm onto CWave fields (#86). Play records intern handle (#96). No OpenAL / sound.cpp.
add_executable(rs2_wave_load_test port/wave_load_test.cpp lib/wave.cpp port/wav_pcm.cpp
  port/path.cpp port/rs2_audio.cpp)
target_include_directories(rs2_wave_load_test SYSTEM BEFORE PRIVATE
  "${CMAKE_SOURCE_DIR}/port/stub")
target_include_directories(rs2_wave_load_test PRIVATE
  "${CMAKE_SOURCE_DIR}"
  "${CMAKE_SOURCE_DIR}/lib"
  "${CMAKE_SOURCE_DIR}/port")
target_compile_features(rs2_wave_load_test PRIVATE cxx_std_17)
target_compile_options(rs2_wave_load_test PRIVATE
  -Wall
  -Wextra
  -Wno-invalid-source-encoding)
target_compile_definitions(rs2_wave_load_test PRIVATE RS2_PORTABLE_COMPILE_FIREWALL=1 NOMINMAX)

add_test(NAME rs2_wave_load_self_test COMMAND rs2_wave_load_test --self-test)
add_test(NAME rs2_wave_load_distribution COMMAND rs2_wave_load_test
  "${CMAKE_SOURCE_DIR}/Distribution")
set_tests_properties(rs2_wave_load_distribution PROPERTIES SKIP_RETURN_CODE 77)

# CheckTexTrans: texture file name -> transparent colour, Windows DWORD values (#182). No D3D.
add_executable(rs2_tex_trans_test port/tex_trans_test.cpp lib/texture.cpp
  port/path.cpp port/rs2_text.cpp)
target_include_directories(rs2_tex_trans_test SYSTEM BEFORE PRIVATE
  "${CMAKE_SOURCE_DIR}/port/stub")
target_include_directories(rs2_tex_trans_test PRIVATE
  "${CMAKE_SOURCE_DIR}"
  "${CMAKE_SOURCE_DIR}/lib"
  "${CMAKE_SOURCE_DIR}/port")
target_compile_features(rs2_tex_trans_test PRIVATE cxx_std_17)
target_compile_options(rs2_tex_trans_test PRIVATE
  -Wall
  -Wextra
  -Wno-invalid-source-encoding
  -Wno-address-of-temporary)
target_compile_definitions(rs2_tex_trans_test PRIVATE RS2_PORTABLE_COMPILE_FIREWALL=1 NOMINMAX)
target_link_libraries(rs2_tex_trans_test PRIVATE Iconv::Iconv)

add_test(NAME rs2_tex_trans_self_test COMMAND rs2_tex_trans_test --self-test)

# InitDirectPlay soft-fails without a DirectPlay8 peer; comm calls then fail (#206).
add_executable(rs2_comm_softfail_test port/comm_softfail_test.cpp lib/comm.cpp)
target_include_directories(rs2_comm_softfail_test SYSTEM BEFORE PRIVATE
  "${CMAKE_SOURCE_DIR}/port/stub")
target_include_directories(rs2_comm_softfail_test PRIVATE
  "${CMAKE_SOURCE_DIR}"
  "${CMAKE_SOURCE_DIR}/lib"
  "${CMAKE_SOURCE_DIR}/port")
target_compile_features(rs2_comm_softfail_test PRIVATE cxx_std_17)
target_compile_options(rs2_comm_softfail_test PRIVATE
  -Wall
  -Wextra
  -Wno-invalid-source-encoding)
target_compile_definitions(rs2_comm_softfail_test PRIVATE RS2_PORTABLE_COMPILE_FIREWALL=1 NOMINMAX)

add_test(NAME rs2_comm_softfail_self_test COMMAND rs2_comm_softfail_test --self-test)

# Stub keyboard/mouse/joy poll + ScanInputDevice merge (#82). No SDL.
# SDL poll lives in rs2_input_sdl.cpp on railsim2_native when RS2_HAVE_SDL2.
add_executable(rs2_input_test port/rs2_input_test.cpp port/rs2_input.cpp)
target_include_directories(rs2_input_test SYSTEM BEFORE PRIVATE
  "${CMAKE_SOURCE_DIR}/port/stub")
target_include_directories(rs2_input_test PRIVATE "${CMAKE_SOURCE_DIR}/port")
target_compile_features(rs2_input_test PRIVATE cxx_std_17)
target_compile_options(rs2_input_test PRIVATE -Wall -Wextra)
target_compile_definitions(rs2_input_test PRIVATE RS2_PORTABLE_COMPILE_FIREWALL=1 NOMINMAX)

add_test(NAME rs2_input_self_test COMMAND rs2_input_test --self-test)

# Win32 message queue: FIFO, WM_QUIT, WndProc dispatch, WM_ACTIVATEAPP on
# create (#205). No SDL: the SDL pump lives in rs2_msg_sdl.cpp on
# railsim2_native when RS2_HAVE_SDL2.
add_executable(rs2_msg_test port/rs2_msg_test.cpp port/rs2_msg.cpp)
target_include_directories(rs2_msg_test SYSTEM BEFORE PRIVATE
  "${CMAKE_SOURCE_DIR}/port/stub")
target_include_directories(rs2_msg_test PRIVATE "${CMAKE_SOURCE_DIR}/port")
target_compile_features(rs2_msg_test PRIVATE cxx_std_17)
target_compile_options(rs2_msg_test PRIVATE -Wall -Wextra)
target_compile_definitions(rs2_msg_test PRIVATE
  RS2_PORTABLE_COMPILE_FIREWALL=1
  NOMINMAX
  RS2_HAVE_SDL2=0)

add_test(NAME rs2_msg_self_test COMMAND rs2_msg_test --self-test)

# Stub PCM buffer intern + play/stop/volume/3D/listener (#93). No OpenAL.
add_executable(rs2_audio_test port/rs2_audio_test.cpp port/rs2_audio.cpp port/wav_pcm.cpp)
target_include_directories(rs2_audio_test PRIVATE "${CMAKE_SOURCE_DIR}/port")
target_compile_features(rs2_audio_test PRIVATE cxx_std_17)
target_compile_options(rs2_audio_test PRIVATE -Wall -Wextra)

add_test(NAME rs2_audio_self_test COMMAND rs2_audio_test --self-test)

# Stub IME / TEXTINPUT composition + result (#100). No SDL / Imm COM.
add_executable(rs2_ime_test port/rs2_ime_test.cpp port/rs2_ime.cpp port/rs2_text.cpp)
target_include_directories(rs2_ime_test PRIVATE "${CMAKE_SOURCE_DIR}/port")
target_compile_features(rs2_ime_test PRIVATE cxx_std_17)
target_compile_options(rs2_ime_test PRIVATE -Wall -Wextra)
target_link_libraries(rs2_ime_test PRIVATE Iconv::Iconv)

add_test(NAME rs2_ime_self_test COMMAND rs2_ime_test --self-test)

# D3DX8 matrix / plane / quaternion / bound math, known values (#204).
add_executable(rs2_d3dx_math_test port/d3dx_math_test.cpp port/d3dx_math.cpp)
target_include_directories(rs2_d3dx_math_test SYSTEM BEFORE PRIVATE
  "${CMAKE_SOURCE_DIR}/port/stub")
target_compile_features(rs2_d3dx_math_test PRIVATE cxx_std_17)
target_compile_options(rs2_d3dx_math_test PRIVATE -Wall -Wextra)
target_compile_definitions(rs2_d3dx_math_test PRIVATE RS2_PORTABLE_COMPILE_FIREWALL=1 NOMINMAX)

add_test(NAME rs2_d3dx_math_self_test COMMAND rs2_d3dx_math_test --self-test)
