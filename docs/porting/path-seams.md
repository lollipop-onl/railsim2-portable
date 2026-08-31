# Path I/O seams (`chdir`, `fopen`, `_findfirst`)

- **Issue**: [#30](https://github.com/lollipop-onl/railsim2-portable/issues/30) (parent [#4](https://github.com/lollipop-onl/railsim2-portable/issues/4); [#10](https://github.com/lollipop-onl/railsim2-portable/issues/10) depends on `CSaveFile::Load`)
- **Tree**: root `*.cpp` / `lib/*.cpp` at this document's commit. Counts exclude `port/stub/` and comments except where noted.
- **This slice does not implement path resolution.** `#4` should replace **this closed set**, not every `fopen` in the tree.

Relative names below are under the install root `g_BaseDir` (see [Install root](#install-root-g_basedir)). That is `Distribution/en/RailSim2/` or `Distribution/jp/RailSim2/` in this repo.

## Corrections to issue #4

| Claim in #4 | Fact |
|-------------|------|
| `chdir` 45 sites | **45 matching lines** in **10** `.cpp` files (that is `#4`'s figure). One is a comment in `CRailBuildMode.cpp`. The other 44 lines contain **62** live `chdir(` calls (`||` chains put several on one line). Capture alone is 17 lines. `CPlugin::ChDir` is three `chdir`s, then **36** call sites. |
| `fopen` 25 sites | **28** `fopen(` tokens in game + `lib/` (4 in `CPlugin.cpp`, 4 in `CSaveFile.cpp`). `InitLanguage` / `CConfigMode::Load` go through `LoadBinaryText(fname)`, which is one of those 28. |
| `CSaveFile::Load` is `chdir(g_BaseDir) \|\| chdir(dirname)` then relative `fopen` | True **only when `auxdata` is null**. Network join passes `auxdata` and skips all file I/O. `Save` also `CheckSlash(fname)` so the file name must be a basename (no `/` or `\`). |

`_chdir` is never called from game code. The stub `#define _chdir chdir` is unused. `SetCurrentDirectory` is only the dead macro `CHANGE_DIR` in `lib/debug.h`. `_getcwd` has **zero** callers.

## Process cwd model

`WakeUp()` (`WakeUp.cpp`) sets `g_BaseDir` via `GetAppPath` -> `GetModuleFileName` + `CutPath`. `CutPath` / `CutFileName` / `GetAppPath` understand **backslash only**. `CheckSlash` already rejects both `/` and `\`.

After that, almost all asset I/O is:

1. `chdir(g_BaseDir)` (and often a subdirectory)
2. `fopen` / `rename` / `remove` / `_findfirst` / `CPixelbit::Save` / `g_TexList.Get` with a **relative** name
3. cwd is left wherever the last `chdir` went (process-global, not restored)

`chdir` returns 0 on success, so `chdir(a) || chdir(b)` means "fail if either move fails". `CPlugin::ChDir` inverts that with `!chdir`.

The mkdir idiom in `CFileMode` / `Capture` is `chdir(g_BaseDir); if (chdir(subdir)) mkdir(subdir);`. A failed `chdir` leaves cwd at `g_BaseDir`, so `mkdir` creates the sibling there.

## Install root (`g_BaseDir`)

| Relative dir / file | Role |
|---------------------|------|
| `Language.txt` | UI strings (`InitLanguage`) |
| `Config.txt` | settings; missing -> in-code defaults (`CConfigMode`) |
| `LackPlugin.txt` | written at install root after a layout load that found missing plugins |
| `Layout/` | `*.rs2` layouts (`LAYOUT_DIRNAME`) |
| `Undo/` | `UndoNN.rs2` (`UNDO_DIRNAME`); created on first `CFileMode` ctor |
| `Picture/` `Video/` | screenshots / frame dumps; created by `InitCapture` |
| `{Env,Girder,Line,Pier,Pole,Rail,Skin,Station,Struct,Surface,Tie,Train}/` | plugin **type** dirs; each child folder is a plugin **ID** |
| `{type}/{id}/{Type}2.txt` | current plugin definition (`TextName2`, e.g. `Rail2.txt`) |
| `{type}/{id}/{Type}.txt` | RS1 old form (`TextName`, some types only) |
| `TrainGroupTemplate/*.txt` | train-group templates (`TGT_DIRNAME`) |
| `RailwayPluginSet/*.txt` | railway plugin-set presets (`RPS_DIRNAME`) |

`Distribution/*/RailSim2` already has the plugin / `Layout` / `Language.txt` trees. `Config.txt`, `Picture/`, `Video/`, and `Undo/` are created at runtime if missing.

## Closed `chdir` set

Ten translation units. Do not add new game files to a `#4` path rewrite.

| File | Function | Sequence | Relative I/O that follows |
|------|----------|----------|---------------------------|
| `CPlugin.cpp` | `CPlugin::ChDir` | `g_BaseDir` / `DirName()` / `m_ID` | See [Plugin `ChDir` riders](#plugin-chdir-riders). This is the **one** helper to replace; do not rewrite each caller. |
| `CPlugin.cpp` | `CPluginList::List` | `g_BaseDir` / `DirName()`, then per entry `g_BaseDir` / `DirName()` / `data.name` | `_findfirst("*")` (subdirs only); `fopen(TextName2())` else `fopen(TextName())` |
| `CPlugin.cpp` | `CPluginList::LoadOne` | `g_BaseDir` only | `fopen(defpath)`. Plugin-viewer argv is a Windows absolute `X:\...\.txt` (`WakeUp.cpp`); a relative `defpath` is under `g_BaseDir`. |
| `CSaveFile.cpp` | `Load` | `g_BaseDir` / `dirname` | `fopen(fname, "rb")`. See [CSaveFile::Load](#csavefileload-for-10). |
| `CSaveFile.cpp` | `Load` (lack log) | `g_BaseDir` | `fopen("LackPlugin.txt", "wt")` |
| `CSaveFile.cpp` | `Save` | `CheckSlash(fname)` then `g_BaseDir` / `dirname` | `fopen(fname, "rb")` existence probe; `fopen(fname, "wt")` |
| `CFileMode.cpp` | ctor | `g_BaseDir`; try `Layout`; try `Undo` | `mkdir` if `chdir` fails |
| `CFileMode.cpp` | `ConfirmRename` | `g_BaseDir` / `Layout` | `rename(old, new)`; on failure `fopen(new)` to distinguish EEXIST |
| `CFileMode.cpp` | delete menu | `g_BaseDir` / `Layout` | `remove(fname)` |
| `CFileMode.cpp` | `ListFile` | `g_BaseDir` / `Layout` | `_findfirst("*.rs2")`; `fopen(data.name)` |
| `CTrainGroupTemplate.cpp` | `DeleteFromDisk` / `Rename` / `LoadTrainGroupTemplateList` / `AddTrainGroupTemplate` | `g_BaseDir` / `TrainGroupTemplate` | `remove` / `rename`; `_findfirst("*.txt")`; `fopen(data.name)` or `"%s.txt"` |
| `CRailwayPluginSet.cpp` | same pattern | `g_BaseDir` / `RailwayPluginSet` | same as templates; `AddRailwayPluginSet` `fopen(fname, "wt")` |
| `CConfigMode.cpp` | `Load` / `Save` | `g_BaseDir` | `LoadBinaryText("Config.txt")`; `fopen("Config.txt", "wt")` |
| `Language.cpp` | `InitLanguage` | `g_BaseDir` | `LoadBinaryText("Language.txt")` |
| `Capture.cpp` | `InitCapture`, F12 still, BMP/AVI count, video start | `g_BaseDir` then `Picture` or `Video` | `mkdir`; `fopen("%08d.bmp" / "%08d.avi")`; `CPixelbit::Save` |
| `SystemCover.cpp` | `MoveToFile` | dirname of its argument (`CutPath`, `\` only) | cwd becomes the mesh file's directory so `CMesh::Load` textures resolve. **One caller:** `CMeshList::Get`. |
| `CRailBuildMode.cpp` | (commented dump) | `g_BaseDir` | `fopen("MultiTrackRailList.txt")` - **not** in the implementation set |

## Plugin `ChDir` riders

`CPlugin::ChDir` is the seam for plugin **assets**. Callers (do not expand `#4` into these files except to keep compiling if the helper signature changes):

`CCustomizerMisc.cpp`, `CEnvPlugin.cpp`, `CGirderPlugin.cpp`, `CLinePlugin.cpp`, `CModelPlugin.cpp`, `CNamedObject.cpp`, `CPierPlugin.cpp`, `CPlugin.cpp` (`SetIconTexture` -> `g_TexList.Get`), `CPolePlugin.cpp`, `CProfilePlugin.cpp`, `CRailPlugin.cpp`, `CSkinPlugin.cpp`, `CStationPlugin.cpp`, `CStructPlugin.cpp`, `CSurfacePlugin.cpp`, `CTiePlugin.cpp`, `CTrainPlugin.cpp`.

Relative `fopen` **after** `ChDir()` (old-form plugins):

| File | Name |
|------|------|
| `CStationPlugin::LoadOldForm` | `Station.txt` |
| `CStructPlugin::LoadOldForm` | `Struct.txt` |
| `CSurfacePlugin::LoadOldForm` | `Surface.txt` |
| `CTrainPlugin::LoadOldForm` | `Train.txt` |

`CAnim::Load` (`lib/anim.cpp`) `fopen(strFile, "r")` is also cwd-relative; plugins `ChDir` first, then load animation sidecars / `Model.x`.

## `_findfirst` / `_findnext` / `_findclose`

Four live sites. The stub in `port/stub/io.h` always returns `-1`, so **all four lists are empty** on the native `check` build. Plugin discovery, the file dialog, templates, and railway sets depend on this.

| File | Function | Pattern | Filter |
|------|----------|---------|--------|
| `CPlugin.cpp` | `CPluginList::List` | `"*"` | keep `_A_SUBDIR`; then `chdir` into `data.name`. Windows also yields `.` and `..`; missing `TextName2` makes those iterations no-ops. A `std::filesystem` port should skip `.` / `..` by name. |
| `CFileMode.cpp` | `ListFile` | `"*.rs2"` | skip subdirs |
| `CTrainGroupTemplate.cpp` | `LoadTrainGroupTemplateList` | `"*.txt"` | skip subdirs; stem becomes the template name |
| `CRailwayPluginSet.cpp` | `LoadRailwayPluginSetList` | `"*.txt"` | same as templates |

No other `_finddata_t` users.

## `_fullpath` / `_getcwd` / `SetCurrentDirectory`

| API | Callers | Notes |
|-----|---------|-------|
| `_fullpath` | `CTexList::Get` (`lib/texture.cpp`), `CMeshList::Get` (`lib/mesh.cpp`), `CWave::Load` (`lib/wave.cpp`) | Intended: resolve `strName` against **cwd** into a cache key. Mesh then `MoveToFile(full)` so texture filenames inside the `.x` load from the mesh directory. |
| `_fullpath` stub | `port/stub/direct.h` | `snprintf` copies `rel` unchanged. Not a real absolute path. `MoveToFile` + `CutPath` then no-op on a name with no `\`. |
| `_getcwd` | **none** | stub `#define _getcwd getcwd` only |
| `SetCurrentDirectory` / `CHANGE_DIR` | **none** (macro only) | stub always returns `TRUE` |
| `GetCurrentDirectory` | `lib/music.cpp` `LoadMusic` | `music.h` is commented out of `lib/udx.h`. Out of scope for `#4` unless music is revived. |

`GetModuleFileName` (used by `GetAppPath`) is **not** in `port/stub/` yet. It only matters once `SystemCover.cpp` is allowlisted.

## Other `fopen` that is not a `#4` rewrite target

These are cwd-relative but already covered by a helper in the closed set, or they take a path the caller already made absolute:

- `LoadBinaryText(fname)` (`CPlugin.cpp`) wraps `fopen(fname, "rb")`. Callers are `InitLanguage` / `CConfigMode::Load` after `chdir(g_BaseDir)`.
- `lib/debug.cpp` `fopen(g_debugDest)` where `g_debugDest` is `GetAppPath` + `debug.txt` (`-dbf`). Treat as absolute once `GetAppPath` works.

Do not walk the rest of the game replacing `fopen` with `std::filesystem` one-by-one. Replace **cwd mutation** (`chdir` / `ChDir` / `MoveToFile`) plus the four `_findfirst` loops plus real `_fullpath`.

## `CSaveFile::Load` (for #10)

Signature (`CSaveFile.cpp`):

```
bool Load(const char *fname, const char *dirname, bool warn, bool upname,
          char **copy, int *copysize, bool checkhash, char *auxdata);
```

Typical layout open from `CFileMode`: `fname` = `Sample.rs2` (basename), `dirname` = `LAYOUT_DIRNAME` (`"Layout"`), `auxdata` = `NULL`.

### Sequence when `auxdata == NULL` (disk)

1. `chdir(g_BaseDir)` - fail -> optional dialog -> `false`. cwd unchanged on that failure if `g_BaseDir` itself is bad; if the first `chdir` succeeded and `dirname` failed, cwd is already `g_BaseDir`.
2. `chdir(dirname)` - `Layout` or `Undo`.
3. `fopen(fname, "rb")` - **relative to that directory**. No `CheckSlash` on Load (unlike Save). An absolute `fname` would still open on POSIX/Windows, but production callers pass a basename.
4. `LoadBinaryText(df)` reads the whole file (script text, not a struct dump; see `#10` / [rs2-roundtrip.md](rs2-roundtrip.md)).
5. Optional MD5 (`checkhash`) and optional byte copy (`copy` / `copysize`).
6. Parse `DatafileHeader` / `LayoutInfo` / ... . `DatafileType` must equal `"Layout"` even when the directory is `Undo`.
7. If `g_LackPlugin` is non-empty: `chdir(g_BaseDir)` then `fopen("LackPlugin.txt", "wt")`. Otherwise **cwd stays in `dirname`**.

### Sequence when `auxdata != NULL` (network)

No `chdir`, no `fopen`. The buffer is `strlen(auxdata)` bytes. `#10` may feed `Sample.rs2` this way to avoid cwd, but `Save` still needs a directory + basename (next paragraph).

### `Save` (same file; `#10` will call it)

1. `CheckSlash(fname)` - **any `/` or `\` -> error 1**. `/tmp/out.rs2` cannot be passed through.
2. `chdir(g_BaseDir) || chdir(dirname)`.
3. If `!overwrite` and `fopen(fname, "rb")` succeeds -> return 2 (exists).
4. `fopen(fname, "wt")` and `fprintf` the script.

`CFileMode` writes layouts as `Save(fname, "Layout", ...)` and undo as `Save("Undo%02d.rs2", "Undo", true, false)`.

### How `#10` should call this (do not implement here)

The ctest harness ([rs2-roundtrip.md](rs2-roundtrip.md)) uses **absolute** in/out paths. `CSaveFile::Save` will reject that out path.

Closed options for the later `#10` slice:

1. Set `g_BaseDir` to `Distribution/en/RailSim2`, `Load("Sample.rs2", "Layout", ...)`, `Save` a basename under `Layout` or a temp sibling, then compare files by absolute path **outside** `CSaveFile`.
2. Pass file bytes as `auxdata` for Load; still `Save` via a basename after `chdir`.
3. Do not teach `Save` to accept `std::filesystem` absolute paths in `#10` unless `#4` has already removed `CheckSlash` + `chdir`. Prefer (1).

`#10` must not rewrite the script parser, `%p` width, MD5, or float format inside this inventory.

## Implemented by #32

`port/path.cpp` joins `g_BaseDir` + subdir + basename. Closed-set `chdir` sites use that join. `CPlugin::ChDir` / `MoveToFile` set a **virtual cwd** (`rs2_chdir`); `fopen` is wrapped to `rs2_fopen` so plugin `Load()` riders stay relative. The four `_findfirst` loops call `rs2_list_dir`. `_fullpath` resolves against the virtual cwd. `CutPath` / `CutFileName` treat `/` like `\`. `GetModuleFileNameA` lives in `port/path.cpp`.

**`#4` must not**: rewrite every plugin `Load()`, implement `timeGetTime`, touch `lib/mutex.h`, or mass-convert sources to UTF-8.

**`#10`** may call `Load` / `Save` as documented above and wire `rs2_layout_roundtrip()`. It must not invent a second path scheme.

WASM: treating `g_BaseDir` as the virtual-FS mount and joining instead of `chdir` is the point of `#4`. This document is the call set that join must cover.

## Stub status (after #32)

| Stub | Behavior | Effect |
|------|----------|--------|
| `_findfirst` / `_findnext` | still `-1` (unused) | four loops use `rs2_list_dir` |
| `_fullpath` | `rs2_fullpath` against virtual cwd | mesh / texture / wave cache keys are absolute |
| `SetCurrentDirectory` | always `TRUE` | unused |
| `GetModuleFileNameA` | exe path (or `rs2_set_module_filename`) | `GetAppPath` / `g_BaseDir` can be filled |

## Verification

Closed-set `chdir` / `_findfirst` should be gone from game `.cpp` except the commented dump in `CRailBuildMode.cpp`. `_fullpath` remains in `lib/` and goes through the stub.

```bash
rg -n --glob '!build/**' --glob '!.git/**' --glob '!Distribution/**' --glob '!port/stub/**' \
  --glob '!docs/**' --glob '!port/path*' \
  '\bchdir\s*\(|\b_chdir\s*\(|SetCurrentDirectory|_findfirst|_findnext|_fullpath|_getcwd'
```

`./scripts/check.sh` must stay green (`rs2_path_self_test` + `rs2_path_distribution`).

## Case sensitivity (#41)

Windows (and some macOS volumes) treat path components as case-insensitive. Linux does not. After #32, asset I/O goes through `port/path.cpp` without rewriting game path strings or renaming Distribution files.

### Inventory (what can break)

| Path | Role | Case risk |
|------|------|-----------|
| `rs2_path_join` + `rs2_fopen` / `rs2_is_dir` / `rs2_chdir` / `rs2_rename` / `rs2_remove` / `rs2_fullpath` | Closed-set open and cwd | **Exact** component spelling. Wrong case fails on Linux. |
| `rs2_list_dir` + `glob_match` | Former `_findfirst` loops | Extension / name match uses `icmp` (case-insensitive), matching typical Win32 `_findfirst` behavior. Returned names keep **on-disk** spelling. |
| Hardcoded `DirName()` / `TextName2()` / `LAYOUT_DIRNAME` / `Language.txt` / `Config.txt` | Install-tree literals | Must match Distribution (or runtime-created) names byte-for-byte. |
| Plugin-relative riders after `CPlugin::ChDir` | `Model.x`, textures, `*.txt` sidecars | Resolved under the virtual cwd; same exact-match rule as `rs2_fopen`. |
| Names from `rs2_list_dir` then re-opened | Layout / plugin / template lists | Safe: reopen uses the spelling the directory walk returned. |

### Distribution check (this repo)

Against `Distribution/en/RailSim2` (and the same tree under `jp/`):

- Top-level type dirs (`Rail`, `Layout`, …), `Language.txt`, and `Layout/Sample.rs2` match the game literals exactly.
- Every plugin folder has `*2.txt` (or old-form `*.txt`) with the expected casing; no sibling names that differ only by case.
- Relative asset refs inside those `.txt` files resolve with exact case under the plugin directory (no “open only via casefold” hits in a scan of `.x` / image / wave names).

Runtime-only dirs (`Undo/`, `Picture/`, `Video/`, `Config.txt`) are created with the same literals the game later opens, so they do not introduce a case gap.

### Policy: keep strict open / chdir

**Do not** add a case-folding resolver in `rs2_fopen` / `rs2_chdir` / `rs2_is_dir`. Evidence:

1. Shipped Distribution and game string literals already agree; a folding walker would be speculative.
2. Issue #41 forbids guessing a large path rewrite; fix the resolution layer only when a real mismatch appears.
3. `rs2_list_dir` already folds pattern matches; callers that open listed names stay correct on Linux.
4. If third-party or hand-edited assets ever disagree in case, prefer fixing the asset or the authoring string over silent folding (folding hides collisions and diverges from “path as written” debugging).

`rs2_path_self_test` creates a probe file and, on a case-sensitive volume, asserts that a wrong-case `rs2_fopen` fails while `*.RS2`-style listing still succeeds. On case-insensitive volumes the strict-fail check is skipped.
