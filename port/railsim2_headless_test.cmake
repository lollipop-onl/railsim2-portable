# cmake -DRAILSIM2=<path> -P railsim2_headless_test.cmake
#
# WinMain returns 0 whether CApp::Init fails or the game runs to the end, so
# the exit status alone cannot say where the run stopped. The debug stream on
# stderr does. It is CP932; only the ASCII stage names Debug() prints on a
# line of their own are matched, anchored to whole lines so that a CP932
# trail byte in the 0x40-0x7E range can never complete a match.
#
# Not PASS_REGULAR_EXPRESSION: ctest then ignores the exit status, and a
# signal would pass as long as the stream got far enough first.

if(NOT RAILSIM2)
  message(FATAL_ERROR "RAILSIM2 is not set")
endif()

execute_process(
  COMMAND "${RAILSIM2}"
  RESULT_VARIABLE status
  OUTPUT_VARIABLE out
  ERROR_VARIABLE err
  TIMEOUT 60
)

set(failures "")

if(NOT status STREQUAL "0")
  string(APPEND failures "exit status is '${status}', expected 0 with no signal\n")
endif()

string(REPLACE "\r\n" "\n" err "\n${err}")

function(stage_index name var)
  string(FIND "${err}" "\n${name}\n" index)
  set(${var} ${index} PARENT_SCOPE)
endfunction()

stage_index(InitDebugStream debug_stream)
stage_index(InitDirect3D d3d_init)
stage_index(InitDirectInput input_init)
stage_index(FreeDirect3D d3d_free)

if(debug_stream EQUAL -1)
  string(APPEND failures "InitDebugStream is missing: CApp::Init did not reach the debug stream\n")
endif()
if(d3d_init EQUAL -1)
  string(APPEND failures "InitDirect3D is missing\n")
endif()
if(NOT input_init EQUAL -1)
  string(APPEND failures "InitDirectInput is present: InitDirect3D succeeded, so CreateDevice no longer fails headless\n")
endif()
if(d3d_free EQUAL -1)
  string(APPEND failures "FreeDirect3D is missing: ~CApp did not run after WinMain returned\n")
elseif(NOT d3d_init EQUAL -1 AND d3d_free LESS d3d_init)
  string(APPEND failures "FreeDirect3D precedes InitDirect3D\n")
endif()

if(failures)
  message(FATAL_ERROR "${failures}--- stderr ---${err}\n--- stdout ---\n${out}")
endif()
