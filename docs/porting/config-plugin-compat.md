# Config.txt / Language.txt / plugin `*2.txt` verification

- **Issue**: [#58](https://github.com/lollipop-onl/railsim2-portable/issues/58) (parent [#10](https://github.com/lollipop-onl/railsim2-portable/issues/10))
- **Sources**: `port/rs2_config_plugin.{h,cpp}`, `port/rs2_config_plugin_test.cpp`
- **ctest**: `rs2_config_plugin_self_test`, `rs2_config_plugin_load`
- **Path rules**: [path-seams.md](path-seams.md)

This slice locks **text-definition** load for settings and one representative plugin header. It does **not** change `.rs2` layout format, expand to every plugin type, or enter M3 / `.x` draw. Layout mechanical gates stay in [rs2-roundtrip.md](rs2-roundtrip.md) / #59.

## Call-site inventory

| Symbol | File | When | Path |
|--------|------|------|------|
| `CConfigMode::Load` | `CConfigMode.cpp` | `CGameMode::WakeUp` after `InitLanguage` | `rs2_path_join(g_BaseDir, "Config.txt")` then `LoadBinaryText`. Missing file Å® in-code defaults (`goto SET`), still `true`. Parse `CSynErr` Å® `false`. Leftover after `ConfigMode{}` is `g_ConfigScript`. |
| `CConfigMode::Save` | `CConfigMode.cpp` | `CGameMode` shutdown when `!g_RSPV` | `fopen(cfgpath, "wt")`. Writes `DatafileHeader` + `ConfigMode` (current `RAILSIM_VERSION` 2.15), then `SaveModeSettings` (per-mode tails). `YESNO[]` is `"no"` / `"yes"`. |
| `CGameMode::LoadModeSettings` | `CGameMode.cpp` | after modes exist | Walks `g_ConfigScript` through each mode `LoadSetting`, including `g_ConfigMode->LoadSetting`. Errors still tagged `"Config.txt"`. |
| `CGameMode::SaveModeSettings` | `CGameMode.cpp` | from `CConfigMode::Save` | Appends per-mode blocks after `ConfigMode{}`. Out of this ctest (not required for settings smoke). |
| `InitLanguage` | `Language.cpp` | `CGameMode::WakeUp` (first) | `LoadBinaryText(rs2_path_join(g_BaseDir, LANG_FILE_NAME))` with `LANG_FILE_NAME = "Language.txt"`. `DatafileType = Language`, then `Language.Name`, then the `Resource{}` string table. |
| `CPlugin::LoadHeader` / `PreLoad` | `CPlugin.cpp` | `CPluginList::List` per folder | `fopen` `{type}/{id}/{Type}2.txt` (`TextName2`, e.g. `Rail2.txt`) else old-form `TextName`. Header only: `PluginHeader` + `PluginType == DirName()`. Full `CPlugin::Load` / meshes stay later. |

`Distribution/*/RailSim2` ships `Language.txt` and plugin `*2.txt`. `Config.txt` is **runtime-created**; first boot uses the defaults path.

## Contract (ctest)

`rs2_config_plugin_self_test`

- Missing `Config.txt` Å® `rs2_config_load_or_defaults` applies the same locals `CConfigMode::Load` uses before `SET` (640x480, `Default_JR_Narrow`, Åc).
- Parse a `CConfigMode::Save`-shaped 2.15 buffer; keep 2.00 buffers valid (no `WindowShadow` / `ShowMap` / `Stereoscopy`, gated defaults remain).
- Write + parse roundtrip of non-default fields (`YESNO` lowercase, `RailSimVersion = %.2f`, `Interval = %.2f`).
- Inline `PluginHeader` + `Language` `DatafileHeader` smoke.

`rs2_config_plugin_load` (`Distribution/`)

- `Language.txt` header (`Name` non-empty).
- `Rail/Default_JR_Narrow/Rail2.txt` `LoadHeader` (`PluginType = Rail`). There is no `Rail/Rail01/` in Distribution; this is the default selected rail ID.
- `Config.txt` if present, else defaults. Missing fixture root Å® exit **77**.

```bash
./scripts/check.sh
# or:
ctest --preset check --output-on-failure -R rs2_config_plugin
```

The harness is a closed Script.cpp-shaped scanner in `port/`. It does not link `CConfigMode` / `CRailPlugin` (UI, sound, mesh). Trailing per-mode settings after `ConfigMode{}` are ignored, matching `g_ConfigScript`.

## Out of scope

- `.rs2` object graph / `%p` / MD5 / float lexemes (#40 / #44 / #50 / #54 / #59)
- Full plugin `Load()` (profiles, `.x`, wav, M3)
- Every `*2.txt` in Distribution
- Rewriting `CConfigMode.cpp` / `Language.cpp`
