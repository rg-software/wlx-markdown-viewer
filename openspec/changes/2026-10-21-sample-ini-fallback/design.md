# Design

## Context

See `proposal.md` for the motivation. The plugin keeps its settings in `MarkdownView.ini` next to the DLL. `InitOptions()` (`HTMLView/functions.cpp`) computes that path from the module file name (`MarkdownView.wlx` / `MarkdownView.wlx64` -> `MarkdownView.ini`) and copies `MarkdownView.ini.sample` over it on first run. The release archive ships the sample only (archived change `preserve-ini-on-update`).

All settings are read with `GetPrivateProfileString` / `GetPrivateProfileInt` against `options.IniFileName`, with a hardcoded default supplied at each call site. The sample file is never read.

## Goals / Non-Goals

**Goals:**

- Make `MarkdownView.ini.sample` the single source of truth for the shipped defaults.
- Resolve every setting through three layers: user INI -> shipped sample -> hardcoded read-site default.
- Keep the hardcoded third layer (a deleted or unreadable sample must not change behaviour).
- Keep the first-run creation of `MarkdownView.ini` as it is.

**Non-Goals:**

- Changing the packaging layout (still ships `MarkdownView.ini.sample`, no `MarkdownView.ini`).
- Dropping the first-run creation of `MarkdownView.ini` (it is still the friendliest behaviour for a writable directory).
- Reworking the INI format, adding new options, or versioning the schema.
- Layering files that are not the plugin configuration: `positions.ini` and `MarkdownView_Log.ini` keep using `GetPrivateProfile*` directly.

## Decisions

1. **One helper, three layers**
   - New module `HTMLView/ini_settings.h` / `HTMLView/ini_settings.cpp` with:
     - `void GetIniSettingString(const char* ini_path, const char* section, const char* key, const char* hardcoded_default, char* buffer, int buffer_size);`
     - `int GetIniSettingInt(const char* ini_path, const char* section, const char* key, int hardcoded_default);`
   - `HTMLView/functions.h` adds thin wrappers over `options.IniFileName`:
     - `void GetSettingString(const char* section, const char* key, const char* hardcoded_default, char* buffer, int buffer_size);`
     - `int GetSettingInt(const char* section, const char* key, int hardcoded_default);`
   - Rationale: one place implements the layering, so no read site can accidentally bypass it. The wrapper keeps the existing call style (`GetSettingInt("options", "ShowToolbar", 1)`).
   - Alternative: extend `CSmallStringList` with a layered loader only. Rejected because most settings are plain strings/ints, not lists.

2. **Detection of "key not found" with a sentinel**
   - `GetPrivateProfileString(section, key, SENTINEL, buffer, size, file)` returns `SENTINEL` verbatim when the key, the section, or the file is missing (verified against the Win32 implementation: the default is returned but never written back to the file, and a missing file is not created).
   - If the sentinel comes back, the same call is repeated against the sample; if it comes back again, the hardcoded default is used.
   - Rationale: a single API call per layer, no extra file parsing, and the Win32 API keeps its case-insensitive section/key matching.
   - The sentinel is 4 control characters (`\x01\x02\x03\x04`); buffers smaller than the sentinel are rejected and fall through to the hardcoded default.

3. **Sample path**
   - The sample is looked up as `<ini_path>.sample`, i.e. `MarkdownView.ini.sample` in the plugin directory - the exact name and location the release archive ships.
   - Rationale: no new configuration knob, and it matches the file `InitOptions()` already copies from.

4. **The sample is authoritative: hardcoded defaults are aligned to the sample**
   - Every *active* (uncommented) key of the sample is mirrored into the hardcoded third layer, so that layer 2 and layer 3 always return the same value.
   - The mirrored values live in one place, `HTMLView/settings_defaults.h`, and are referenced by name at the read sites (`DEFAULT_SHOW_TOOLBAR`, `DEFAULT_MARKDOWN_EXTENSIONS`, ...). A single header keeps the mirror auditable and lets the test compare it with the shipped sample.
   - Rationale: the sample is what the user sees and what every user with a writable directory gets (first-run copy). If the hardcoded defaults disagreed with it, adding layer 2 would silently change the behaviour of users whose INI lacks a key - exactly the drift this change removes. Aligning the code (not the sample) is the smallest possible delta: it changes only the "neither file" case, whereas changing the sample would change the first-run copy for every new user.
   - Consequence: with the sample present and the hardcoded defaults in sync, the effective configuration does not depend on whether the sample file is present. Deleting the sample degrades to the mirrored defaults instead of to "no extensions, no renderer arguments".
   - Alternative: drop the hardcoded defaults for keys the sample provides. Rejected - the third layer is kept deliberately so a deleted sample cannot break the plugin.

5. **Commented-out sample keys stay inactive**
   - The `[Debug]` keys (`UseMozillaControl`, `QiuckQuit`, `GlobalHook`, `ToolbarBPP`, `KeepHookWhenNoWindows`) are commented out in the sample and therefore resolve to their hardcoded defaults (`0`, `0`, `0`, `2`, `0`), exactly as today.

6. **Dynamic key space (`[Hotkeys]` action map)**
   - The plugin's own hotkeys are read by key name (`GetSettingString("Hotkeys", full_name, "", action, sizeof(action))`), so there is no fixed per-key default to hardcode. The third layer for that key space stays "no action".
   - Rationale: enumerating the sample's ~20 hotkey entries in C++ would duplicate the map for no benefit - the map is only useful together with the sample that documents it. The two *static* hotkey lists (`TranslationHotkeys`, `TypingTranslationHotkeys`) are mirrored like every other static key.

7. **Integer conversion parity**
   - `GetIniSettingInt` reproduces `GetPrivateProfileInt` semantics: an empty value yields the default, everything else is converted with base 0 (`0x10` -> 16, ` 5 ` -> 5, `abc` -> 0), and the result is truncated to 32 bits.
   - Rationale: the values are used as flags (`if(...)`, `3 & value`, `!value`), so the conversion must not change meaning. `abc` -> 0 (not the default) is what the Win32 API does today.

8. **Empty values are values**
   - An existing key with an empty value (`ListerTitle=`) is found in layer 1 and wins - it is not treated as "missing". The Win32 API returns the empty string, not the default, in that case.
   - Rationale: the sample documents "Empty string means not to change title"; an empty value must not silently fall through to another layer.

9. **The "file not found" warning only fires when there is no configuration at all**
   - `InitOptions()` shows the warning only when neither `MarkdownView.ini` nor `MarkdownView.ini.sample` exists. A read-only plugin directory (INI cannot be created) is covered by the sample and therefore stays silent.
   - Rationale: the layered read makes a missing INI a supported situation; warning about it would be noise. The warning still points at a genuinely broken installation.

10. **The mirrored defaults are verified by the test**
   - `tests/ini_settings_test.cpp` reads the shipped `Build/MarkdownView.ini` and compares every active key with `HTMLView/settings_defaults.h`; a mismatch fails the test.
   - Rationale: the duplication between the sample and the third layer is deliberate, so it needs an automatic drift guard next to the audit table below.

## Audit table: sample value vs. hardcoded default

Every *active* sample key against the hardcoded default at its read site. "after" is the value the third layer uses after this change; "match" means no change was needed.

### `[options]`

| Key | Sample value | Hardcoded default before | Hardcoded default after | Read site |
| --- | --- | --- | --- | --- |
| `ShowDirs` | `1` | `0` | `1` | `functions.cpp` `InitOptions` |
| `ListerTitle` | *(empty)* | `""` | `""` | `functions.cpp` `InitOptions`, `browserhost.cpp` `UpdateTitle` |
| `NewWindowHTML` | `default` | `""` | `"default"` | `browserhost.cpp` `DISPID_NEWWINDOW` |
| `NewWindowCHM` | `default` | `""` | `"default"` | `browserhost.cpp` `DISPID_NEWWINDOW` |
| `AllowPopups` | `0` | `0` | `0` | `functions.cpp` `InitOptions` |
| `ShowStatusbar` | `1` | `0` | `1` | `functions.cpp` `InitOptions` |
| `ShowToolbar` | `1` | `0` | `1` | `functions.cpp` `InitOptions` |
| `UseSavePosition` | `0` | `0` | `0` | `functions.cpp` `InitOptions` |
| `Silent` | `1` | `1` | `1` | `functions.cpp` `InitOptions` |
| `AllowScripting` | `0` | `0` | `0` | `functions.cpp` `InitOptions` |
| `ShowImages` | `1` | `1` | `1` | `functions.cpp` `InitOptions` |
| `ShowVideos` | `0` | `1` | `0` | `functions.cpp` `InitOptions` |
| `PlaySounds` | `0` | `1` | `0` | `functions.cpp` `InitOptions` |
| `AllowJava` | `0` | `0` | `0` | `functions.cpp` `InitOptions` |
| `AllowActiveX` | `1` | `0` | `1` | `functions.cpp` `InitOptions` |
| `ForceOffline` | `1` | `1` | `1` | `functions.cpp` `InitOptions` |
| `HighlightAllMatches` | `0` | `0` | `0` | `functions.cpp` `InitOptions` |

### `[Extensions]`

| Key | Sample value | Hardcoded default before | Hardcoded default after | Read site |
| --- | --- | --- | --- | --- |
| `CHMExtensions` | `chm` | `""` | `"chm"` | `main.cpp` `InitProc` |
| `HTMLExtensions` | `htm;html;shtml;plg;mht;mhtml;xml;xsl;php;php2;php3;xhtml;pl;asp;htt` | `""` | same as sample | `main.cpp` `InitProc` |
| `MarkdownExtensions` | `md;markdown;mdown;mdtext;mdtxt;mdwn;mk;mkd;mkdn;mkdown` | `""` | same as sample | `main.cpp` `InitProc` |
| `DefaultSignatures` | `""<HTML>";"<?xml";"<!DOCUMENT";"<!DOCTYPE HTML""` | `""` | same as sample | `main.cpp` `InitProc` |
| `SignatureSkipSpaces` | `1` | `0` | `1` | `functions.cpp` `InitOptions` |

### `[Hotkeys]`

| Key | Sample value | Hardcoded default before | Hardcoded default after | Read site |
| --- | --- | --- | --- | --- |
| `TranslationHotkeys` | `1;2;3;4;5;6;7;N;P;F3;Shift+F3;F7;Esc;Tab;Ctrl+Q;Ctrl+P;Ctrl+A;Ctrl+C;Ctrl+Insert;Ctrl+F` | `""` | same as sample | `main.cpp` `InitProc` |
| `TypingTranslationHotkeys` | `F3;Shift+F3;F7;Esc;Tab;Ctrl+Q;Ctrl+P;Ctrl+A;Ctrl+C;Ctrl+Insert;Ctrl+F` | `""` | same as sample | `main.cpp` `InitProc` |
| *(per-key actions, e.g. `Z=Back`)* | `Back`, `Forward`, `Stop`, ... | `""` | `""` (dynamic key space, see decision 6) | `browserhost.cpp` `ProcessHotkey` |

### `[Renderer]`

| Key | Sample value | Hardcoded default before | Hardcoded default after | Read site |
| --- | --- | --- | --- | --- |
| `HoedownArgs` | `hoedown --hard-wrap --all-block --autolink --strikethrough --highlight --superscript` | `""` | same as sample | `main.cpp` `InitProc` |
| `UseSmartypants` (read as `UseSmartyPants`, matched case-insensitively) | `1` | `""` | `"1"` | `main.cpp` `InitProc` |
| `CustomCSS` | `css\github.css` | `""` | `"css\\github.css"` | `main.cpp` `InitProc` |
| `CustomCSSDark` | `css\github.dark.css` | `""` | `"css\\github.dark.css"` | `main.cpp` `InitProc` |

### `[Debug]` (all keys commented out in the sample - inactive)

| Key | Sample value | Hardcoded default before | Hardcoded default after | Read site |
| --- | --- | --- | --- | --- |
| `UseMozillaControl` | *commented out* | `0` | `0` | `functions.cpp` `InitOptions` |
| `QiuckQuit` | *commented out* | `0` | `0` | `functions.cpp` `InitOptions` |
| `GlobalHook` | *commented out* | `0` | `0` | `functions.cpp` `InitOptions` |
| `ToolbarBPP` | *commented out* | `2` | `2` | `functions.cpp` `InitOptions` |
| `KeepHookWhenNoWindows` | *commented out* | `0` | `0` | `functions.cpp` `InitOptions` |

## Risks / Trade-offs

- [Existing INI missing a key] Such a user now inherits the key from the sample instead of from the old hardcoded default. After the alignment above the two are equal, so this only affects keys whose sample value changed since the user's INI was written - that is the intended "fallback base" behaviour.
- [Duplication] The third layer duplicates the sample's values in C++. This is deliberate (decision 4); the audit table above is the drift guard and must be updated together with the sample.
- [Sample deleted] The plugin keeps working with the mirrored defaults, but the hotkey action map is lost (decision 6).
- [Read failures] A missing or unreadable file at any layer simply falls through to the next layer; nothing is logged and no message box is shown.
- [`GetPrivateProfileString` side effects] None: the Win32 API does not write the default value back into the file and does not create a missing file (verified).

## Migration Plan

1. Add `HTMLView/ini_settings.*`, `HTMLView/settings_defaults.h` and the wrappers, route all setting reads through them.
2. Align the hardcoded defaults with the audit table above.
3. Add `tests/ini_settings_test.cpp` covering the 2x2 matrix and every key of the audit table.
4. Build Release for Win32 and x64, run the test for both architectures, and load the plugin in Total Commander.
