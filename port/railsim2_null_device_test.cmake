# cmake -DRAILSIM2=<path> -DDATA_SOURCE=<dir> -DDATA_COPY=<dir>
#       -P railsim2_null_device_test.cmake
#
# Starts railsim2 on the recording device (RS2_NULL_DEVICE=1) against a
# fresh copy of DATA_SOURCE, and has it post WM_QUIT from the
# QUIT_AFTER-th Present on. The stream is matched the way
# railsim2_headless_test.cmake matches it, and for the same reasons: whole
# ASCII lines only, and not PASS_REGULAR_EXPRESSION.
#
# Present is only reached from EndScene, inside Main's frame loops, so a
# recorded Present is what says CApp::Init returned TRUE and the game got
# past it. Clear alone would not: InitRenderState clears once inside
# InitDirect3D.

foreach(var RAILSIM2 DATA_SOURCE DATA_COPY)
  if(NOT ${var})
    message(FATAL_ERROR "${var} is not set")
  endif()
endforeach()

set(QUIT_AFTER 3)

file(REMOVE_RECURSE "${DATA_COPY}")
get_filename_component(copy_parent "${DATA_COPY}" DIRECTORY)
file(MAKE_DIRECTORY "${copy_parent}")
file(COPY "${DATA_SOURCE}/" DESTINATION "${DATA_COPY}")

execute_process(
  COMMAND "${CMAKE_COMMAND}" -E env
    RS2_NULL_DEVICE=1
    "RS2_DATA_DIR=${DATA_COPY}"
    RS2_QUIT_AFTER_FRAMES=${QUIT_AFTER}
    "${RAILSIM2}"
  RESULT_VARIABLE status
  OUTPUT_VARIABLE out
  ERROR_VARIABLE err
  TIMEOUT 120
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
if(input_init EQUAL -1)
  string(APPEND failures "InitDirectInput is missing: InitDirect3D failed on the recording device\n")
elseif(input_init LESS d3d_init)
  string(APPEND failures "InitDirectInput precedes InitDirect3D\n")
endif()
if(d3d_free EQUAL -1)
  string(APPEND failures "FreeDirect3D is missing: ~CApp did not run after WinMain returned\n")
elseif(d3d_free LESS input_init)
  string(APPEND failures "FreeDirect3D precedes InitDirectInput\n")
endif()

string(REGEX MATCH "\nrs2_null_device clears=([0-9]+) presents=([0-9]+)\n" record "${err}")
if(NOT record)
  string(APPEND failures "rs2_null_device record is missing: the device was not released to zero\n")
else()
  set(clears ${CMAKE_MATCH_1})
  set(presents ${CMAKE_MATCH_2})
  string(FIND "${err}" "${record}" record_index)
  if(NOT d3d_free EQUAL -1 AND record_index LESS d3d_free)
    string(APPEND failures "the device was released before FreeDirect3D\n")
  endif()
  if(clears LESS 1)
    string(APPEND failures "no Clear was recorded\n")
  endif()
  if(presents LESS ${QUIT_AFTER})
    string(APPEND failures "${presents} Presents recorded, expected at least ${QUIT_AFTER}: the frame loop ended before RS2_QUIT_AFTER_FRAMES\n")
  endif()
endif()

if(failures)
  message(FATAL_ERROR "${failures}--- stderr ---${err}\n--- stdout ---\n${out}")
endif()
