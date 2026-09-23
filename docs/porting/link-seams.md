# Link seams (`railsim2_native` linked into an executable)

- **Issue**: [#190](https://github.com/lollipop-onl/railsim2-portable/issues/190) (parent [#189](https://github.com/lollipop-onl/railsim2-portable/issues/189))
- **Measured**: 2026-09-23 at `4521f44`, on both hosts below. Every count here is a count at that commit, not a property of the tree. Re-measure before relying on one.
- **This is an inventory, not a design.** It records what the linker reports when the game objects are linked, so that #189's follow-up slices can be cut from a measured list. It does not pick the fixes.

| Host | Compiler | Linker |
|------|----------|--------|
| macOS 26.7, arm64 | AppleClang 21.0.0 | ld64 (`ld-27037.1`) |
| Linux (docker `rs2check:local`, Ubuntu 24.04, aarch64) | Ubuntu clang 18.1.3 | GNU ld 2.42 |

The Linux numbers come from an aarch64 container, not from CI's x86_64 `ubuntu-24.04` runner. Nothing in the symbol set is expected to depend on the architecture, but x86_64 was not measured; the linker's message text and offsets do differ.

## Summary

| Link | macOS | Linux |
|------|------:|------:|
| whole archive (`-force_load` / `--whole-archive`), empty `main` | 3 | 3 |
| whole archive, `main` calls `WinMain` | 3 | 3 |
| plain archive link, `main` calls `WinMain` | 1 | 1 |

The numbers are **distinct undefined symbols**, not linker message lines. ld64 folds repeated references into `...`, and GNU ld stops listing a symbol after a few lines with `more undefined references to ... follow`, so neither message count means anything. On both hosts the set the linker named matched the set computed from `nm` (below): symbols the archive references, minus symbols it defines, minus the C / C++ runtime and `iconv`.

- **Strong duplicate definitions: 0** on both hosts. No two members of `railsim2_native.a` define the same strong global.
- **Win32 / DirectX / COM / GUID / CRT names: 0.** Every API in `port/stub/` is an inline function or a `#define` in a header, so no call ever reaches the linker as an undefined symbol. This is why the list is short: the stubs already resolve at compile time what a Windows build resolves at link time.

## Undefined symbols by kind

| Kind | Symbol | Referenced from | Defined where | Plain link | Whole archive |
|------|--------|-----------------|---------------|:----------:|:-------------:|
| udx `SYSVALUE_*` global | `svm` (`SYSVALUE_M`) | `lib/music.cpp` only (`InitDirectMusic`, `FreeDirectMusic`, `FreeMusic`, for example) | nowhere: `lib/sysvalue.h:18` has the definition commented out | not reported | reported |
| udx `SYSVALUE_*` global | `svv` (`SYSVALUE_V`) | `lib/movie.cpp` only (`InitDirectShow`, `FreeDirectShow`, for example) | nowhere: `lib/sysvalue.h:19` has the definition commented out | not reported | reported |
| `port/` function | `rs2_cmesh_probe_xfile(const char*, Rs2XMesh*, std::string*)` | `lib/mesh.cpp:174`, `CMesh::Load`, the `RS2_PORTABLE_COMPILE_FIREWALL` branch added in #45 | `port/rs2_cmesh_xfile.cpp`, which needs `port/xfile.cpp`; at `4521f44` both are compiled into `rs2_cmesh_xfile_test` only | reported | reported |

`svm` and `svv` are the only two `SYSVALUE_*` globals that `lib/sysvalue.h` does not define; `lib/comm.h:48-49` carries the same two lines in a comment block. Upstream's `RailSim2.vcxproj` did not compile `lib/music.cpp` or `lib/movie.cpp` at all, so upstream never had to define them. The allowlist compiles every game TU, which is what brings the two references in.

`rs2_cmesh_probe_xfile` is the only undefined symbol a plain link reports. `CMesh::Load` is reachable from `WinMain`, so `lib/mesh.cpp` is always pulled, and its firewall branch calls into `port/` code that `railsim2_native` did not contain at `4521f44`.

Adding the missing definitions closes the list on both hosts at this commit: a scratch object defining `SYSVALUE_M svm;` and `SYSVALUE_V svv;`, plus `port/rs2_cmesh_xfile.cpp` and `port/xfile.cpp`, made both the plain and the whole-archive link succeed. That is a check that nothing hides behind the three -- a linker reports every undefined symbol, not the first one -- and not a proposal for where the definitions belong.

### Since measured

2026-09-23: #191 added `port/xfile.cpp` and `port/rs2_cmesh_xfile.cpp` to `railsim2_native` (`707c0b6`). Re-measured on macOS only, at that commit: a plain link from a `main` that calls `WinMain` now succeeds and the headless run below still exits 0, and a whole-archive link reports `svm` and `svv` alone. #195 tracks those two. The rest of this document is still the `4521f44` measurement.

2026-09-23: #197 defined `svm` and `svv` (`88177c0`). #198 then linked `railsim2` from `railsim2_native` whole (`$<LINK_LIBRARY:WHOLE_ARCHIVE,...>`) with `port/native_entry.cpp` calling `WinMain`, and replaced the `CMakeLists.txt` comment described next. On both hosts `railsim2` links with no undefined symbols and contains the 12 TUs listed below (`InitDirectMusic` from `lib/music.cpp` and `InitDirectShow` from `lib/movie.cpp`, for example). With #196's `CreateDevice` failure, the headless run now stops in `InitDirect3D`: the stream shows `InitDebugStream`, `InitDirect3D`, `FreeInput`, `FreeDirect3D`, and the process still exits 0.

### `CMakeLists.txt`'s comment is stale

The comment above `add_executable(railsim2 ...)` says the game objects "still need udx globals (sv3, g_frame, ...) from lib/". They do not. `lib/sysvalue.h` defines them (`sv3` at `:13`, `g_frame` at `:22`), and `lib/main.cpp` is the TU that includes it, so any link that pulls `lib/main.cpp` has them. The only udx globals still missing are `svm` and `svv` above.

## Which members a plain link pulls

`railsim2_native.a` has 165 members at this commit: the 152 game TUs and 13 `port/` objects. A plain link from a `main` that calls `WinMain` pulls 152 of them on both hosts (read from the ld64 `-map` / GNU ld `-Map` output). The 13 it leaves behind are 12 game TUs and `port/rs2_input_sdl.cpp`:

| TU | In upstream's `RailSim2.vcxproj` |
|----|:-------------------------------:|
| `CPixelbitStamp.cpp` | yes |
| `stdafx.cpp` | yes |
| `lib/anim.cpp` | yes |
| `lib/effect.cpp` | yes |
| `lib/view_ctrl.cpp` | yes |
| `lib/wave_stream.cpp` | yes |
| `lib/height_field.cpp` | no |
| `lib/movie.cpp` | no |
| `lib/music.cpp` | no |
| `lib/particle.cpp` | no |
| `lib/sprite.cpp` | no |
| `lib/water_mesh.cpp` | no |

Nothing reachable from `WinMain` references a symbol these TUs define, so an archive link has no reason to open them. The six `lib/` files upstream never compiled are all in this list, and `lib/movie.cpp` / `lib/music.cpp` are exactly the TUs that hold the `svv` / `svm` references. That is why the plain link reports one symbol and the whole-archive link three.

#189's completion condition is that `railsim2` links all 152 TUs. A plain link does not do that, so the condition needs whole-archive linking or its equivalent (an object library, or `$<TARGET_OBJECTS:...>`), and with it the two globals.

## Entry point

`lib/main.cpp:40` is the game's entry:

```cpp
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT){
```

Under `port/stub/windows.h`, `WINAPI` is empty, `HINSTANCE` is `void*`, `LPSTR` is `char*` and `INT` is `int`. `WinMain` is therefore an ordinary C++ function with C++ linkage, not Windows' `extern "C"` `__stdcall` `_WinMain@16`. Both hosts mangle it the same way, apart from Mach-O's leading underscore:

| Host | Symbol |
|------|--------|
| macOS | `__Z7WinMainPvS_Pci` |
| Linux | `_Z7WinMainPvS_Pci` |

A POSIX `main` links against it by declaring the same prototype after including the stub `<windows.h>`, which is what the measurement did:

```cpp
#include <windows.h>
INT WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, INT);
int main() { return WinMain(nullptr, nullptr, (LPSTR)"", 0); }
```

The caller's TU needs `port/stub` on its include path. An `extern "C"` declaration would not match. `port/native_entry.cpp` at `4521f44` includes nothing and returns 0.

### `__argc` / `__argv`

The game reads its command line through the MSVC CRT globals `__argc` / `__argv`, not through `WinMain`'s `LPSTR`. The stub maps them to inline functions (`port/stub/windows.h:861-866`):

```cpp
inline int rs2_argc() { return 0; }
inline char** rs2_argv() { return nullptr; }
#define __argc rs2_argc()
#define __argv rs2_argv()
```

Both readers loop from `i = 1` while `i < __argc`, so neither loop body runs and every option is silently ignored:

- `CheckArguments` (`lib/debug.cpp:71`), which serves, for example, `-dbf` (`lib/debug.cpp:10`), `-2nd` / `-win` (`lib/graphic.cpp`), `/3ds` / `/fx` (`lib/sound.cpp`) and `-voodoo` (`CStringTexture.cpp:275`).
- `WakeUp` (`WakeUp.cpp:26`), which takes the first argument shaped like `X:\...txt` as `g_PluginViewArg`. The drive-letter test (`cparam[1]==':'`, `cparam[2]=='\\'`) never matches a POSIX path, independently of the stub.

Because the stub's accessors are inline and return constants, a `main` has nowhere to store `argc` / `argv` for the game to read. Passing them through needs storage the stub can reference. `port/stub/io.h` and `port/stub/direct.h` push, undefine and pop both macros around `<unistd.h>`, because glibc uses `__argv` as a parameter name there; whatever replaces the accessors has to keep that working.

## Why `rs2_roundtrip`'s stubs are not reusable

`rs2_roundtrip` links 31 real game TUs with the `port/rs2_roundtrip_*.cpp` files. Most of those files stand in for game TUs the harness leaves out, so they define game symbols. Against the whole of `railsim2_native.a`, which contains the real TUs, they collide. Strong global definitions in common, the same on both hosts:

| Roundtrip object | Strong definitions also defined in `railsim2_native.a` |
|------------------|-----:|
| `rs2_roundtrip_stubs.cpp` | 214 |
| `rs2_roundtrip_seams.cpp` | 133 |
| `rs2_roundtrip_plugins.cpp` | 128 |
| `rs2_roundtrip_readmap.cpp` | 2 |
| `rs2_roundtrip_ptr.cpp` | 0 |
| **total** | **477** |

What carries over is the pattern, not the files: a missing global is defined in a `port/` TU, which is how `port/rs2_roundtrip_stubs.cpp` supplies globals such as `g_NetworkInitialized` for the harness.

## Headless run

Both linked scratch executables (plain and whole-archive, with the missing definitions added) were started with no arguments on both hosts. Neither creates a window. This is a reference observation for #189's smoke-test slice, not a test.

`CApp::Init` (`lib/main.cpp`) runs, and the debug stream on `stderr` shows each stage in order:

```
InitDebugStream
InitDirect3D
InitDirectInput
InitDirectSound
InitDirectPlay
FreeInput
FreeDirect3D
```

`InitDirectPlay` (`lib/comm.cpp`) calls `CoCreateInstance`, which the stub answers with `E_NOTIMPL` (`port/stub/windows.h:559`). `InitDirectPlay` returns `FALSE`, `lib/main.cpp:112` returns `FALSE` from `CApp::Init`, and `WinMain` returns 0. The process exited with status 0 on both hosts, with nothing on `stdout`. [network-seams.md](network-seams.md) predicted this path before anything was linked. `WinMain` returns 0 whether `CApp::Init` fails or the game runs to the end, so the exit status alone cannot tell a smoke test which of the two happened.

Two things in that run are not what a smoke test should pin:

- **UBSan reports member calls on a null `IDirect3DDevice8`.** A macOS Debug build with `-fsanitize=address,undefined` reported 19 `runtime error: member call on null pointer of type 'IDirect3DDevice8'` lines in one run -- in `lib/graphic.cpp`, `lib/light.cpp`, `lib/render.h` and `lib/texture.h`, for example -- and still exited 0. The stub's `IDirect3D8::CreateDevice` (`port/stub/d3d8.h:427`) returns `S_OK` without writing its `IDirect3DDevice8**` out-parameter, so `sv3.pDev` stays null and `InitDirect3D` goes on to call methods through it. The calls do not crash only because the stub's methods do not dereference `this`. The Linux run was not sanitized.
- **The logged adapter mode and caps change from run to run.** `InitDirect3D` prints values such as the current display mode and the maximum texture size that differ between runs on the same host, including negative widths. They are read from structs that the stub's query methods do not fill. Output comparison against this log would be flaky.

## Mapping to #189's follow-up slices

#189 cuts one slice per kind. At `4521f44` the kinds mapped as follows; the first row has since been resolved. Slice numbers are not assigned here; see #189 for the current checklist.

| Kind | Needed for | What the slice has to decide |
|------|------------|------------------------------|
| `port/` function (`rs2_cmesh_probe_xfile`) | any link, plain or whole | Resolved by #191: both files joined `railsim2_native` (see "Since measured" above) |
| udx `SYSVALUE_*` globals (`svm`, `svv`) | whole-archive link only | Whether the two definitions go into a `port/` TU, or into `lib/sysvalue.h` (a game-source edit, which AGENTS.md Hard constraint 1 requires to be mechanical or covered by a `check` test) |
| Whole-archive linking | "all 152 TUs linked" | How the 12 TUs above are forced in on both linkers, and that the link stays free of strong duplicates |
| Entry (`WinMain`, `__argc` / `__argv`) | a `railsim2` that runs the game | How `port/native_entry.cpp` reaches `WinMain`, and whether `argc` / `argv` reach `CheckArguments` in the same slice |
| Headless smoke test | #189's completion condition | What the test asserts. The exit status and the absence of a signal are stable at this commit, but the status does not say which stage stopped `CApp::Init`. The log's device values are not stable, and the null-device calls are a stub defect a test should not pin |

The stale `CMakeLists.txt` comment belongs to whichever slice next edits the `railsim2` target.

## Reproducing the measurement

The experiment adds scratch targets to a copy of the tree and never touches the checkout. `W` is any scratch directory.

```bash
W=/path/to/scratch
mkdir -p "$W/src"
git archive 4521f44 | tar -x -C "$W/src"
mkdir -p "$W/src/linkexp"
cat > "$W/src/linkexp/trivial_main.cpp" <<'EOF'
int main() { return 0; }
EOF
cat > "$W/src/linkexp/winmain_main.cpp" <<'EOF'
#include <windows.h>
INT WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, INT);
int main() { return WinMain(nullptr, nullptr, (LPSTR)"", 0); }
EOF
cat > "$W/src/linkexp/svm_svv.cpp" <<'EOF'
#include "headers.h"
#include <dshow.h>
#include "debug.h"
#include "window.h"
#include "graphic.h"
#include "sound.h"
#include "movie.h"
#include "music.h"
SYSVALUE_M svm;
SYSVALUE_V svv;
EOF
```

Keep the tree at `4521f44`. From `707c0b6` on, `railsim2_native` already contains `port/rs2_cmesh_xfile.cpp`, so `linkexp_whole_closed` (the closure objects plus the force-loaded archive) would define `rs2_cmesh_probe_xfile` twice.

Append these targets to `$W/src/CMakeLists.txt`. `whole_*` link the archive whole, `entry_*` link it plainly, and `*_closed` add the missing definitions:

```cmake
if(APPLE)
  set(RS2_WHOLE "-Wl,-force_load,$<TARGET_FILE:railsim2_native>")
else()
  set(RS2_WHOLE "-Wl,--whole-archive" "$<TARGET_FILE:railsim2_native>" "-Wl,--no-whole-archive")
endif()
set(RS2_LINKEXP_DEFS RS2_PORTABLE_COMPILE_FIREWALL=1 NOMINMAX)

add_library(linkexp_closure OBJECT port/rs2_cmesh_xfile.cpp port/xfile.cpp)
target_include_directories(linkexp_closure PRIVATE "${CMAKE_SOURCE_DIR}/port")
add_library(linkexp_svmsvv OBJECT linkexp/svm_svv.cpp)
target_include_directories(linkexp_svmsvv PRIVATE "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/lib" "${CMAKE_SOURCE_DIR}/port")
target_compile_options(linkexp_svmsvv PRIVATE -Wno-invalid-source-encoding -Wno-address-of-temporary)

add_executable(linkexp_whole_trivial linkexp/trivial_main.cpp)
add_executable(linkexp_whole_winmain linkexp/winmain_main.cpp)
add_executable(linkexp_entry_winmain linkexp/winmain_main.cpp)
add_executable(linkexp_entry_closed linkexp/winmain_main.cpp $<TARGET_OBJECTS:linkexp_closure>)
add_executable(linkexp_whole_closed linkexp/winmain_main.cpp
  $<TARGET_OBJECTS:linkexp_closure> $<TARGET_OBJECTS:linkexp_svmsvv>)

foreach(_t linkexp_closure linkexp_svmsvv linkexp_whole_trivial linkexp_whole_winmain
           linkexp_entry_winmain linkexp_entry_closed linkexp_whole_closed)
  target_include_directories(${_t} SYSTEM BEFORE PRIVATE "${CMAKE_SOURCE_DIR}/port/stub")
  target_compile_definitions(${_t} PRIVATE ${RS2_LINKEXP_DEFS})
endforeach()
foreach(_t linkexp_whole_trivial linkexp_whole_winmain linkexp_whole_closed)
  add_dependencies(${_t} railsim2_native)
  target_link_libraries(${_t} PRIVATE ${RS2_WHOLE} Iconv::Iconv)
endforeach()
foreach(_t linkexp_entry_winmain linkexp_entry_closed)
  target_link_libraries(${_t} PRIVATE railsim2_native Iconv::Iconv)
endforeach()
if(APPLE)
  target_link_options(linkexp_entry_closed PRIVATE "-Wl,-map,${CMAKE_BINARY_DIR}/entry_closed.map")
else()
  target_link_options(linkexp_entry_closed PRIVATE "-Wl,-Map,${CMAKE_BINARY_DIR}/entry_closed.map")
  foreach(_t linkexp_whole_trivial linkexp_whole_winmain linkexp_entry_winmain linkexp_whole_closed)
    target_link_options(${_t} PRIVATE -Wl,--unresolved-symbols=report-all)
  endforeach()
endif()
```

Build each target on its own, so that one failing link does not stop the next. The three that are expected to fail print their undefined symbols; the two `*_closed` targets are expected to link.

```bash
B="$W/build"   # the same commands run inside the Linux container with B=/work/build
cmake -S "$W/src" -B "$B" -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=clang++
ninja -C "$B" railsim2_native
for t in whole_trivial whole_winmain entry_winmain entry_closed whole_closed; do
  ninja -C "$B" linkexp_$t > "$W/link-$t.log" 2>&1; echo "$t rc=$?"
done
```

The symbol set, the duplicate check and the member list. The `awk` on `nm -u` accepts both formats (macOS prints bare names, GNU prints `U name`). `nm`'s letter does not separate strong from weak definitions on Mach-O (an inline function is `T` there too), so `strong_defs` reads `nm -m` on macOS and the letter on Linux. The two blocks after this one run in the same shell, reusing `LC_ALL`, `A` and `strong_defs`:

```bash
export LC_ALL=C   # sort, comm and uniq have to agree on collation
strong_defs() {
  if [ "$(uname)" = Darwin ]; then
    nm -m -g --defined-only "$@" 2>/dev/null | grep ' external ' |
      grep -v 'weak\|private external' | awk '{print $NF}'
  else
    nm -g --defined-only "$@" 2>/dev/null | awk 'NF>=3 && $2~/^[TDBRC]$/{print $3}'
  fi
}
A="$B/librailsim2_native.a"

nm -g --defined-only "$A" 2>/dev/null | awk 'NF>=3{print $3}' | sort -u > "$W/def.txt"
nm -g -u "$A" 2>/dev/null | awk '$1=="U"{print $2} NF==1 && $1!~/:$/{print $1}' | sort -u > "$W/und.txt"
comm -23 "$W/und.txt" "$W/def.txt" > "$W/unresolved.txt"   # then drop libc / libc++ / libm / iconv by eye

strong_defs "$A" | sort | uniq -d                          # strong duplicates: expected empty

ar t "$A" | grep -v SYMDEF | sort > "$W/all-members.txt"   # 165 at 4521f44
```

The members a plain link pulled come from the link map of `linkexp_entry_closed`, whose layout differs by linker. Run it in the same shell as the block above:

```bash
M="$B/entry_closed.map"
if [ "$(uname)" = Darwin ]; then
  sed -n '/^# Object files:/,/^# Sections:/p' "$M"
else
  sed -n '/^Archive member included/,/^Discarded input sections/p' "$M"
fi | grep -ao 'librailsim2_native\.a([^)]*)' | sed 's/.*(//;s/)//' | sort -u > "$W/pulled.txt"
comm -23 "$W/all-members.txt" "$W/pulled.txt"             # the 13 left out at 4521f44
```

For the roundtrip collision count, build `rs2_roundtrip` in the same build directory and intersect strong definitions, again in the same shell:

```bash
ninja -C "$B" rs2_roundtrip
strong_defs "$A" | sort -u > "$W/strong.txt"
for o in "$B"/CMakeFiles/rs2_roundtrip.dir/port/rs2_roundtrip_*.o; do
  echo "${o##*/} $(strong_defs "$o" | sort -u | comm -12 - "$W/strong.txt" | wc -l)"
done
```

On Linux, pipe the prepared tree into the container instead of bind-mounting it. A bind mount from macOS leaks case-insensitivity into the container and hides case bugs. The container checks itself: `ls RSPV.H` has to fail.

```bash
COPYFILE_DISABLE=1 tar --no-xattrs --no-mac-metadata -c -C "$W/src" . |
  docker run --rm -i rs2check:local bash -c '
    set -u
    mkdir -p /work/src && cd /work/src && tar -x
    if ls RSPV.H >/dev/null 2>&1; then echo "case-insensitive tree" >&2; exit 99; fi
    W=/work B=/work/build
    # then the cmake / ninja / nm blocks above, with W/src read as /work/src
  '
```

The headless run is `"$B/linkexp_entry_closed"` and `"$B/linkexp_whole_closed"` with no arguments; the debug stream goes to `stderr`. For the sanitizer run, configure a second build directory with `-DCMAKE_CXX_FLAGS='-fsanitize=address,undefined -fno-omit-frame-pointer' -DCMAKE_EXE_LINKER_FLAGS='-fsanitize=address,undefined'` and count with `grep -ac 'runtime error'`. The debug stream is CP932; `grep -a` keeps it from being skipped as binary.
