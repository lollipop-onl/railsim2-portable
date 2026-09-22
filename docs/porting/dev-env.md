# Mac / CI development environment

M0 goal: one non-interactive command (`./scripts/check.sh`) that matches CI.

## Prerequisites

Install tools with mise (recommended):

```bash
mise install          # reads .mise.toml (cmake, ninja, ccache)
./scripts/check.sh    # auto-adds mise tool paths when needed
```

Legacy fallback (Homebrew):

```bash
brew bundle --file Brewfile
```

| Tool | Purpose |
|------|---------|
| cmake | Configure presets |
| ninja | Native builds (objects + linked `railsim2` stub) |
| ccache | Optional compile cache (wired later) |

## Commands

| Command | What it does |
|---------|----------------|
| `./scripts/check.sh` | Gate: encoding guard -> configure -> build (link) -> ctest -> progress JSON |
| `./scripts/encoding-guard.sh` | CP932 / BOM / SJIS-0x5C literal checks |
| `./scripts/progress.sh` | Prints `N/152` JSON from `port/native_sources.txt` |
| `cmake --preset check` | Configure AppleClang native target |
| `cmake --build --preset check` | Compile allowlisted sources and link `railsim2` |
| `ctest --preset check` | Smoke + `Sample.rs2` roundtrip + text `.x` load + path join (see [rs2-roundtrip.md](rs2-roundtrip.md), [x-file-parser.md](x-file-parser.md), [path-seams.md](path-seams.md)) |
| `cmake --preset runtime` | Same sources, plus **optional** SDL2 / OpenGL / OpenAL search. Not CI. |

## Compile firewall

Windows/DirectX headers are stubbed under `port/stub/` and injected with `-isystem port/stub` **before** system includes. Game code keeps `#include <d3d8.h>` etc.; only the include path changes.

Native targets are listed in `port/native_sources.txt`. CMake compiles those into `railsim2_native.a` and links a stub `railsim2` from `port/native_entry.cpp` (`main` returns 0). Game objects are **not** linked into the executable yet; they still need udx globals from `lib/`. SDL2 is not required for the `check` preset.

Progress denominator = every **game translation unit**: root-level `*.cpp` (124) + `lib/*.cpp` (28) = **152**.
`scripts/progress.sh` counts the tree rather than carrying the number, so the definition
cannot drift from the docs the way `254` did. Adding a line to the allowlist is monotonic progress.

The denominator was **254** until #141: root-level `*.cpp` + `*.h`. Headers are not
translation units, so 130 of those 254 -- every root `*.h` the count saw -- could
never enter the numerator, and `254/254` was unreachable. The numerator meanwhile
counted `lib/*.cpp` too (19 entries at `#141`), which the old denominator excluded --
the two sides were measuring disjoint sets.
`254` was also already stale as a count of its own definition: it was taken when
`RSPV.H` still had an uppercase suffix and so missed a `*.h` glob, and #127 renaming it
to `RSPV.h` made the same rule yield 255. The ratio therefore jumped from `140/254`
(55%) to `140/152` (92%) at #141 without any TU being added.

`port/native_sources.txt` is the only place a game source belongs. `CMakeLists.txt`
also names sources on `railsim2_native` directly, but those are `port/` backing
code, outside the denominator. A game source named in both places compiles once
and correctly -- CMake accepts a duplicate without a warning and the generator
collapses it by object name -- yet `scripts/progress.sh` counts allowlist lines
and nothing else, so the file goes missing from the numerator. `lib/movie.cpp`
sat that way from `dfd45ed` (`#76`) until `#145`, because the allowlist held only
root-level entries back then and `lib/` had nowhere else to go. It is no longer
a literal in `CMakeLists.txt`.

`stdafx.h` already parses through the stubs. Include-order and MSVC-only extra qualification are no longer what blocks the allowlist. The TUs outside it were sorted into four groups; the header comment of `port/native_sources.txt` is the current summary of what still holds them, and `scripts/progress.sh` counts how many are left:

| Blocker | Examples |
|---------|----------|
| Missing D3D8 / DirectX header or type in `port/stub/` | `lib/mesh.cpp` had no `rmxfguid.h` / `rmxftmpl.h` anywhere in the tree, so both hosts stopped at a `fatal error` (closed in `#150`) |
| Missing GDI / Win32 UI types | `CPixelbit.cpp` (`BITMAPFILEHEADER`, `ReadFile`), `lib/font.cpp` (`LOGFONT`, `DT_*`), `lib/texture.cpp`, `lib/sprite.cpp` (`::SetRect` not in stub) |
| Missing DirectSound notify / DirectMusic / DirectPlay8 declarations | None left. `lib/wave_stream.cpp` (closed by `#164`), `lib/comm.cpp` (closed by `#169`, see [network-seams.md](network-seams.md)) and `lib/music.cpp` (closed by `#174`) sat here. This row used to read "Needs a real backend, not a stub" and named `lib/comm.cpp` and `lib/sound.cpp`. Neither belonged: `#152` closed `lib/sound.cpp` with stub declarations alone, and `#124` counts all three TUs here as stub work -- its target is `152/152` with no exceptions, and no TU waits on a backend |
| Type mismatch, nothing missing | None left. `lib/draw.cpp` (initializer-list narrowing, closed by `#161`) and `CWaveArray.cpp` (MSVC array-new bound expression, closed by `#162`) sat here, and both needed a game-source edit rather than a stub; `CWaveArray.cpp` was a parse failure, not a type mismatch |

A TU can sit in more than one row, so the rows are not a partition and the table
alone does not tell you what a stub addition buys. `lib/height_field.cpp` used to
appear under both a missing type and a type mismatch: `IDirect3DTexture8::GetLevelDesc`
closed only 1 of its 3 errors, and the other 2 needed the `min` / `max` seam below.
`lib/texture.cpp` wants `GetLevelDesc` too, yet is still blocked: it went 21 errors
to 18, all of them under the GDI gap.

The row a TU sits in is a guess until it is measured, and three of them were wrong.
`lib/debug.cpp` sat under GDI because of `OSVERSIONINFO`, but that type and the
`GetVersionEx` / `OutputDebugString` beside it are OS-information and debug-output
seams with no GDI in them; it needed one struct and two inline seams, not #16.
`lib/main.cpp` was never in this table, but #124 listed it beside `lib/comm.cpp` /
`lib/music.cpp` / `lib/sound.cpp` as needing a real backend. It needed `CoInitialize` and
`CoUninitialize`, 2 errors against their 70 / 41 / 14, and no COM runtime at all:
a portable build has nothing to initialize, so both are no-ops. `lib/mesh.cpp` was
in the right row but its cost was guessed too high: `#124` read "no `rmxfguid.h`"
as a choice between transcribing the SDK's GUIDs and template table or replacing
the `.x` reader outright, and it was neither. The TU needs the two headers to
*parse*, and the only identifiers it takes from them -- `TID_D3DRMMesh`,
`D3DRM_XTEMPLATES`, `D3DRM_XTEMPLATE_BYTES` -- are read on the `CMesh::Load(fRes=TRUE)`
path alone, which has no caller: all 16 `fRes` arguments in the tree are `FALSE`
(15 `CMeshList::Get` calls and `lib/anim.cpp`'s direct `CMesh::Load`), and the
`FALSE` branch already goes through `port/xfile.cpp` (`#45`). Placeholders with
the reason written on them were enough. Read "needs a real backend" -- and any
estimate of what a row costs -- as a claim to re-measure rather than a verdict.

`OutputDebugString` is the one seam here that is not a bare no-op. It writes to
`stderr`, because `lib/debug.cpp` only reaches it when `g_debugDest` is empty --
that is, when the run has no `-dbf` log file -- so discarding it would silently
drop the default debug output. `<cstdio>` is already included by
`port/stub/windows.h`, so this adds no dependency to the `check` preset.

Before allowlisting a group, compile its TUs with
`-ferror-limit=0` against the flags in `build/check/compile_commands.json` and
check that the list of errors goes to zero, not just that the first one
disappears. Count with `grep -E 'error:'`: `': error:'` misses `fatal error:`,
and CP932 sources need `grep -a` or they are skipped as binary.
Measure in stages: an error can hide a second one behind it. `lib/debug.cpp`
reported 4 errors naming two identifiers, and adding `OSVERSIONINFO` alone
exposed a third one, `GetVersionEx`, that the `unknown type name` diagnostic on
`ver` had been suppressing. A `fatal error` hides everything after it, so a TU
that stops on a missing header has to be measured at least twice: `lib/mesh.cpp`
reported 1, then 18 with the two headers present but empty, then 16 once they
declared their three identifiers -- and the three that went away were replaced by
one that had been unreachable, `invalid operands to binary expression ('const
GUID' and 'const GUID')`, which only exists once `TID_D3DRMMesh` is something to
compare against.

When a stub struct grows, put the new member at its upstream SDK position
rather than at the end -- `d3d8types.h` for most of `port/stub/d3d8.h`, but
`d3d8caps.h` for `D3DCAPS8` and `d3d8.h` itself for `D3DADAPTER_IDENTIFIER8`.
Which members are kept is decided by what the tracked sources read; the order
they are kept in is the part of the layout the tree can get wrong without
anyone noticing, because nothing takes `offsetof`, nothing initializes one of
these with a positional aggregate initializer, and `check` passes either way.
`D3DSURFACE_DESC` held `MultiSampleType` after `Width` and `Height` from the
first stub commit (`#15`) until `#140`.

"Reordering is invisible" holds for the `d3d8.h` structs, not for stub structs
in general. `D3DMATERIAL8` in `port/stub/d3dx8.h` is copied by layout:
`rs2_ffp_set_material` in `port/ffp_state.cpp` memcpys the game's
`D3DMATERIAL8` into `Rs2FfpMaterial` (`port/ffp_state.h`), a field-for-field
twin declared in terms of `float`. Until `#147` the `static_assert` in
`port/ffp_state_test.cpp` compared the two sizes only, so reordering
`D3DMATERIAL8` left `check` green and handed the FFP the wrong colours:
swapping `Ambient` and `Specular` was measured and all 29 tests still passed.
`#147` added one `offsetof` per member, and that swap now stops the build.
`rs2_ffp_set_transform` copies `D3DXMATRIX` the same way, into the snapshot's
`float[16]` transform slots. Order needs no assert there -- `_11 ... _44` name
their own positions, so nobody has a reason to move them -- but the length was a
bare `16 * sizeof(float)` that nothing compared against `sizeof(D3DXMATRIX)`.
`#153` took the length from the destination slot and pinned the two sizes
against each other, so gaining or losing a member stops the build too.
Neither memcpy takes `sizeof` of the stub type it copies: the material one is
`sizeof(g_snap.material)`, the port's own `Rs2FfpMaterial`, and the transform
one is the destination slot's. Everywhere else the tree sizes one of these
stubs, it is clearing or copying the stub type into itself and member order
cannot matter -- `ZeroMemory(&sv3.d3dpp, ...)` in `lib/graphic.cpp`
(`D3DPRESENT_PARAMETERS`, `port/stub/d3d8.h`), `sizeof(D3DLIGHT8)` in
`lib/light.cpp` and `sizeof(MAT8)` (a `D3DMATERIAL8` typedef from
`lib/headers.h`) in `lib/mesh.cpp` and `CCustomizerMisc.cpp`, both from
`port/stub/d3dx8.h`. Order matters only where the bytes cross into a port-side
twin, which is these two functions, and `port/ffp_state_test.cpp` is the only
place in the tree that names `sizeof(D3DMATERIAL8)` or `sizeof(D3DXMATRIX)` to
object.

The same rule applies to the shape of a stub function: the version the game was
written against is **DX8**, and a signature copied from DX9 compiles until the
first call site arrives. Three in `port/stub/` were DX9-shaped and only found
out when `lib/mesh.cpp` -- their sole caller anywhere in the tree -- was
measured at `#150`: `DirectXFileCreate` took `(GUID*, IDirectXFile**)` where DX8
takes the out-parameter alone, and `D3DXLoadMeshFromXof` and
`D3DXComputeBoundingBox` carried DX9's extra `ppEffectInstances` and DX9's
`D3DXVECTOR3*` first parameter in place of DX8's `const void* pPointsFVF`.
`D3DXLoadMeshFromX` beside them is still DX9-shaped and was left alone, because
its only call site sits in the `#else` of `RS2_PORTABLE_COMPILE_FIREWALL` in
`lib/mesh.cpp` and never compiles here -- there is no measurement that could
confirm a change to it.

Width counts too, and a stub type can be wrong in a way no TU will ever report.
`HRESULT` was `long` until `#152`. On Windows it is 32 bits; on LP64 `long` is
64, which leaves every `0x8.......` constant positive and makes `FAILED()`
answer **false** for `E_FAIL`, `E_NOTIMPL` and every `DSERR_*`. Nothing failed
to compile, so the only symptom was that soft-fail paths written against
`FAILED()` -- `InitDirectSound`, and the `InitDirectShow` one `port/stub/dshow.h`
documents -- would have run on as if the call had succeeded. `#152` made it
`int`. When a stub constant is a bit pattern rather than a small number, check
that the type carrying it is as wide as the SDK's, not just as wide as the
value.

### No game TU compiles as-is any more -- but the stubs are far from spent

`lib/movie.cpp` was the last game TU that compiled with the stubs exactly as
they stood, and the allowlist has caught up with it. Measuring every game
`*.cpp` outside the allowlist under `railsim2_native`'s own flags left twelve at
`#145`, and **none of them was at zero errors on either host** (macOS / Linux):

| TU | macOS | Linux | Gap |
|----|------:|------:|-----|
| `CWaveArray.cpp` | 1 | 1 | MSVC array-new bound expression (closed by `#162`) |
| `lib/mesh.cpp` | 1 | 1 | `fatal error`, no `rmxfguid.h` anywhere in the tree (closed by `#150`) |
| `CPixelbitStamp.cpp` | 3 | 3 | GDI |
| `lib/sprite.cpp` | 6 | 4 | GDI (`::SetRect`) |
| `lib/font.cpp` | 9 | 9 | GDI |
| `lib/draw.cpp` | 14 | 14 | initializer-list narrowing (closed by `#161`) |
| `lib/sound.cpp` | 14 | 14 | DirectSound (closed by `#152`) |
| `lib/wave_stream.cpp` | 16 | 16 | DirectSound notify (closed by `#164`) |
| `lib/texture.cpp` | 18 | 18 | GDI |
| `CPixelbit.cpp` | 40 | 40 | GDI |
| `lib/music.cpp` | 41 | 40 | DirectMusic (closed by `#174`) |
| `lib/comm.cpp` | 70 | 68 | DirectPlay8 (closed by `#169`) |

No TU is blocked on one host and clear on the other. The three rows whose
counts differ do so because of clang's typo correction, not because the gap
itself differs. Where the compiler guesses a nearby name it keeps parsing, and
the recovery raises errors of its own: AppleClang reads `::SetRect` as "did you
mean simply `SetRect`", which turns each of the two call sites in
`lib/sprite.h` into a second, bogus arity error; it reads `IID_IDirectSound` as
`InitDirectSound` in `lib/music.cpp` and drags in a `const GUID` binding error;
and it reads `PDPNMSG_RECEIVE` as `PFN_RECEIVE` in `lib/comm.cpp`, producing
two "not a structure or union". **At those three sites** Linux clang 18.1.3
finds no candidate and stops at the undeclared name.

Neither compiler is the one that corrects -- the candidate sets differ site by
site. A few lines away in the same `lib/music.cpp`, Linux offers `_CS_PATH` for
`MAX_PATH` and AppleClang offers nothing. Count the identifiers a TU is
missing, not the diagnostics.

This table said in `#145` that "the stubs already in the tree have nothing left
to offer", and that is the claim to keep; the heading it sat under overshot it
into "the stubs have nothing left to offer", which is not the same sentence and
is not true. `#150` took `lib/mesh.cpp` off this table with two new stub headers
and six declarations, and no game-source diff at all. `#152` then took
`lib/sound.cpp` off it with twelve more declarations in `port/stub/dsound.h`,
and `#164` took `lib/wave_stream.cpp` off it with the seven DirectSound notify
names it was still missing plus `CopyMemory`. `#169` took `lib/comm.cpp` off it
with the DirectPlay8 names in `port/stub/dplay8.h`, `lstrlen`, and a `WCHAR`
that is `wchar_t`, as `winnt.h` has it: the stub's `unsigned short` was why
libc's `mbstowcs` did not match. `#174` took `lib/music.cpp` off it with the
DirectMusic names in `port/stub/dmusici.h`, `IDirectSound`, `CLSCTX_INPROC`,
`CP_ACP` and `GetCurrentDirectory`, plus the `MAX_PATH` and
`MultiByteToWideChar` that `port/stub/dshow.h` had carried with no caller in
`lib/movie.cpp`. `GetCurrentDirectory` and `MultiByteToWideChar` never showed
up in the first measurement: every call to them had an undeclared `MAX_PATH`
as an argument, and clang does not report a callee it cannot resolve the
arguments of.

`#152` is also the first slice where growing a stub **broke** an allowlisted TU.
`#86` had given `lib/wave.cpp` a local copy of the DirectSound names it needed,
guarded by `#ifndef DSBUFFERDESC`. A guard like that can only ever protect
macros, and `DSBUFFERDESC` is a typedef, so declaring it in `port/stub/dsound.h`
turned the local copy into `typedef redefinition with different types`. Before
adding a name to a stub, grep the allowlisted TUs for it: a `#ifndef` around one
does not mean someone checked that it fires.

What the rows are really made of, measured rather than inferred from the Gap
column: **ten of the twelve are missing declarations** -- an absent header, an
undeclared identifier, an unknown type name, a member a stub struct does not
carry -- and only two are the compiler refusing code it has fully understood.
`CWaveArray.cpp` fails to parse (`new (CWave[m_Number = n])`, an assignment in an
array-new bound, is MSVC-only) and `lib/draw.cpp` narrows `int` into a `float`
initializer list fourteen times. **Those two were the only rows that could not be
answered from `port/stub/`**, and at `#145` they were the only ones held for
agreement on editing game code. `#157` removed the need for that agreement by
grounding Hard constraint 1 in `AGENTS.md` on behavior preservation, and
`#161` / `#162` shipped both edits. Every TU still outside the allowlist is a
question of how much stub, not of whether a stub can do it -- the GDI rows
(`#16`) are large, not categorically different, and `#124` records that a TU
can be allowlisted as soon as the stub satisfies its types, without waiting for
`#5` / `#7` / `#11` to have a backend. A stub can also hold a TU by declaring a name in the wrong shape
rather than leaving it out; the header of `port/native_sources.txt` names the
TUs where that is the case today.

Nothing constructs `CWaveStream`, and nothing outside `lib/music.cpp` includes
`music.h` (`lib/udx.h` has it commented out). `#152` briefly took
`lib/wave_stream.cpp` and `lib/music.cpp` off the target on those grounds
(capping it at `150/152`), and parked `lib/comm.cpp` until `network-seams.md`
existed; `#124` withdrew all three, and `#164` then allowlisted
`lib/wave_stream.cpp` with no caller in sight. `#169` allowlisted
`lib/comm.cpp` in the same slice that wrote the first `network-seams.md`, and
`#174` allowlisted `lib/music.cpp` with its header still commented out.
Whether a TU has a caller is a question for the backend work, not for whether
it compiles.

So what is gone is the free sweep, not the headroom. Re-measuring after a stub
grows is still worth doing -- that is how `#126` found 52 TUs at once -- but it
no longer finds a TU that someone else's stub happened to finish.

### `min` / `max` under `NOMINMAX`

`port/stub/windows.h` defines `NOMINMAX` **and** declares global `min` / `max`
function templates. That is not a contradiction: `NOMINMAX` suppresses the Win32
*macros*, and the templates give back the call-site semantics the game was
written against without reintroducing macro text.

The game calls `min` / `max` on mixed types -- `min(m_width-1, max(0, x/m_scale+m_width/2))`
in `CHeightField::GetHeight` mixes `int` and `float`. `std::min` deduces one
parameter type from both arguments and rejects that, while the MSVC `windef.h`
macros expanded to a conditional operator and applied the usual arithmetic
conversions. The stub templates return `std::common_type<A, B>::type`, which is
the conditional operator's result type, so both the value and the type match what
the game saw on MSVC.

The comparison direction is the macros' (`a < b ? a : b`), not `std::min`'s
(`b < a ? b : a`). It matters once a comparison is unordered: MSVC's
`min(NaN, x)` is `x`, `std::min(NaN, x)` is `NaN`. Checking both templates
bit-for-bit against the macro expansion over NaN, infinity and signed-zero
arguments, the macro direction agrees on all of them and `std::min`'s disagrees
on 12 of 32.

Do not turn them back into macros. Commit `59c3b3e` removed the macro versions
because Linux CI could not compile libstdc++ `<limits>`: `windows.h` includes
standard headers first, libstdc++'s `bits/c++config.h` has already run its own
`#undef min` / `#undef max` by then, and a TU that reaches `<limits>` afterwards
parses `numeric_limits::min()` as a function-like macro invocation. Measured on
`ubuntu:24.04` / clang 18.1.3, a macro version fails **149 of the 152 game TUs**
(18072 errors, mostly `limits:1658: too few arguments provided to function-like
macro invocation`). macOS / libc++ compiles the same macro version with no
regression at all, so this is one of the cases where a single host proves
nothing.

Do not delete them as redundant with `std::min` / `std::max` either -- see above.
`using namespace std;` leaks in from several game headers, but there is no
ambiguity: for same-type arguments `std::min<T>(const T&, const T&)` wins by
partial ordering, and for mixed types only `::min` is viable.

One blind spot comes with them. The macros compared at the call site, so
`min(some_int, some_unsigned)` used to raise `-Wsign-compare` there; the
templates compare inside `port/stub/windows.h` after casting both operands to
`common_type`, so the same call is now silent. Two separate things hide it: the
casts make the comparison same-signedness, and `-isystem port/stub` would
suppress a diagnostic in the template body anyway (a cast-free version of the
same template warns under `-I` and is silent under `-isystem`). Today the only
mixed-type calls in the tree are the two `int` / `float` ones in
`CHeightField::GetHeight`, so nothing is being hidden yet -- but a sign-mixed
comparison can now enter game code without any diagnostic, where before it was
a hard error.

### Both hosts, always

CI (`.github/workflows/check.yml`) is a matrix of `macos-15` **and** `ubuntu-24.04`. A TU that compiles on the host you happen to be sitting at is not allowlistable; it has to pass on both.

The rule is not theoretical. Two host asymmetries have been found and fixed, and they pointed in **opposite** directions -- one was fatal only on Linux, the other only on macOS. Either one, measured on the wrong host alone, would have been recorded as a pass. Both are resolved; the traps below outlive them.

#### Include path case -- was fatal on Linux only

`CGameMode.cpp` and `RailSim2.cpp` spelled the include `"RSPV.h"` while the tracked file was `RSPV.H` -- the only `.H` in a tree of 130-plus `.h` headers. macOS resolved it on its case-insensitive filesystem and emitted only `-Wnonportable-include-path`; Linux gave `fatal error: 'RSPV.h' file not found`. The file is now tracked as `RSPV.h` and both TUs are allowlisted.

Two things outlive the fix:

- The `check` preset compiles `railsim2_native` with `-Werror=nonportable-include-path`, so a new case mismatch fails on macOS too instead of waiting for Linux CI. A sweep of every game TU (124 root `*.cpp` plus 28 under `lib/`) is clean under it, allowlisted or not.
- A Docker **bind mount** from a macOS host leaks that case-insensitivity into the container, so a mismatch passes there and the verification proves nothing. Unpack `git archive HEAD` inside the container (overlayfs) and confirm with `ls RSPV.h` / `ls RSPV.H` that the container filesystem really is case-sensitive before measuring.

#### Comparison-operator constness -- was fatal on macOS only

`CLensFlare.cpp` and `CRailwayPluginSet.cpp` called `std::list::sort` on element types whose `operator<` was not `const`-qualified. libc++ invokes the comparator through `const` references, so the operator dropped out of overload resolution (`no matching function for call to 'std::__less<void, void>'`); libstdc++ and MSVC both accepted it. Adding `const` to the two declarations fixed it, and both TUs are allowlisted.

The trigger is the standard library, not the operating system. Linux clang with `libc++-dev` installed and `-stdlib=libc++` reproduces it exactly (`invalid operands to binary expression ('const CFlareElement' and 'const CFlareElement')`); macOS was the only failing leg because it is the only leg that builds against libc++ by default.

What outlives the fix: every `operator<` in the game headers is now `const`-qualified (10 declarations, 12 `.sort()` call sites), so this class of failure is closed rather than merely worked around. A new comparison operator added without `const` reopens it, and no compiler warning catches that -- clang has no `-W` flag for it, and clang-tidy's `readability-make-member-function-const` is a separate tool the build does not run. The mechanical backstop is having a CI leg that builds with **libc++**, which today means the macOS leg. A `-stdlib=libc++` leg on Linux would catch it just as well.

Win32 `.rc` is skipped on native. Icons stay as files until a later loader. Sources stay CP932 (M0 encoding-guard). `stdafx.h` already uses `lib/udx.h` with forward slashes.

## Compiler choice

Primary: **AppleClang** (`check` preset).

- Game root string literals are ASCII; UI text comes from `Language.txt`.
- CP932 comments are allowed via `-Wno-invalid-source-encoding`.
- Two debug strings that contained SJIS trail byte `0x5C` were reworded to ASCII in `lib/debug.cpp` and `lib/graphic.cpp`.

Fallback: Homebrew GCC with CP932 input charset (`check-gcc` preset). Use when AppleClang reports source-encoding issues:

```bash
brew install gcc   # uncomment in Brewfile first
cmake --preset check-gcc
cmake --build --preset check-gcc
ctest --preset check-gcc --output-on-failure
```

## MinGW cross-compile (not supported)

We do **not** vendor DirectX SDK headers or adopt MinGW for CI. The compile-firewall path gives early compile signal without license or SDK packaging issues. Manual MinGW experiments are out of scope for M0.

## CI

GitHub Actions runs `./scripts/check.sh` on a **macOS + Linux matrix** (`macos-15`, `ubuntu-24.04`). Both jobs use `mise install` for cmake/ninja; Linux also installs `clang` from apt. Local Mac dev can use the same path via `.mise.toml`, or legacy `brew bundle`. Do **not** apt-install SDL2, OpenGL loaders, or OpenAL Soft on that job.

## check vs runtime

`check` is the gate. `runtime` is a local-only configure that **may** find window / GL / audio packages. Later GL link, SDL input, and OpenAL play slices use `runtime`; they must not add those packages to `check` or to CI apt.

| Preset | Who runs it | SDL2 / OpenGL / OpenAL |
|--------|-------------|------------------------|
| `check` (`./scripts/check.sh`, CI) | Everyone | **Not searched.** `RS2_RUNTIME=OFF`. Stubs only. |
| `runtime` | Local machine with optional packages | `find_package` **QUIET / not REQUIRED**. Missing package → feature off, configure still succeeds. |

This slice does **not** draw, poll SDL events, or play OpenAL. `cmake --preset runtime` only records `RS2_HAVE_SDL2` / `RS2_HAVE_OPENGL` / `RS2_HAVE_OPENAL` (`ON` or `OFF` in the configure log). Do not link `lib/graphic.cpp` / `lib/vertex.cpp` / `lib/sound.cpp` from this preset yet.

Optional local packages (Homebrew; not in `Brewfile`, not required for `check`):

```bash
brew install sdl2 openal-soft
# OpenAL Soft is keg-only on macOS; prefix it if FindOpenAL misses it:
cmake --preset runtime --fresh \
  -DCMAKE_PREFIX_PATH="$(brew --prefix openal-soft)"
```

Linux: install distro `libsdl2-dev`, OpenGL headers, and `libopenal-dev` the same way — locally, never as a `check` CI step. If nothing is installed, `cmake --preset runtime` must still succeed with all three features off.

See [adr-backend.md](adr-backend.md) for why the stack is SDL2 + GL 3.3 core / GLES3 + OpenAL Soft.

## Next milestones

- **#10** replaces the roundtrip passthrough with `CSaveFile` and drives byte-identity (`%p` / MD5 / float). The ctest harness itself is [#24](https://github.com/lollipop-onl/railsim2-portable/issues/24) ([rs2-roundtrip.md](rs2-roundtrip.md)).
- **#6** replaces `CXFile` / `D3DXLoadMeshFromX` with the closed parser in `port/xfile.cpp`. Loading Distribution `.x` into memory is [#28](https://github.com/lollipop-onl/railsim2-portable/issues/28) ([x-file-parser.md](x-file-parser.md)).
- Backend / window bring-up uses the `runtime` preset for optional SDL2 / GL / OpenAL (see `adr-backend.md`); keep `check` stub-only.
