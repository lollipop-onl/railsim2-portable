# cmake -DRAILSIM2=<path> -DNM=<nm> -P railsim2_whole_archive_test.cmake
#
# Nothing reachable from WinMain references lib/music.cpp or lib/movie.cpp,
# so a plain archive link drops them and still links and runs; the headless
# start cannot tell the difference (docs/porting/link-seams.md). They are also
# the TUs that needed svm / svv, so they are the ones worth pinning.

if(NOT RAILSIM2 OR NOT NM)
  message(FATAL_ERROR "RAILSIM2 and NM must both be set")
endif()

execute_process(
  COMMAND "${NM}" -g --defined-only "${RAILSIM2}"
  RESULT_VARIABLE status
  OUTPUT_VARIABLE symbols
  ERROR_VARIABLE err
)
if(NOT status STREQUAL "0")
  message(FATAL_ERROR "${NM} failed with '${status}':\n${err}")
endif()

set(missing "")
foreach(entry
    "lib/music.cpp=_Z15InitDirectMusicv"
    "lib/movie.cpp=_Z14InitDirectShowv")
  string(REPLACE "=" ";" entry "${entry}")
  list(GET entry 0 tu)
  list(GET entry 1 symbol)
  if(NOT symbols MATCHES " T _?${symbol}\n")
    string(APPEND missing "${tu} (${symbol})\n")
  endif()
endforeach()

if(missing)
  message(FATAL_ERROR "railsim2 does not define symbols from:\n${missing}")
endif()
