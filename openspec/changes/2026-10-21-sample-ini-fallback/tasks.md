# Tasks

## 1. Layered read helper

- [x] 1.1 Add `HTMLView/ini_settings.h` / `HTMLView/ini_settings.cpp` with `GetIniSettingString` / `GetIniSettingInt`: user INI -> `<ini_path>.sample` -> hardcoded default, using a sentinel default to detect a missing key
- [x] 1.2 Add the `GetSettingString` / `GetSettingInt` wrappers over `options.IniFileName` to `HTMLView/functions.h` / `HTMLView/functions.cpp`
- [x] 1.3 Register the new files in `HTMLView/MarkdownView.vcxproj` and `HTMLView/MarkdownView.vcxproj.filters`

## 2. Route all setting reads through the helper

- [x] 2.1 `HTMLView/functions.cpp` `InitOptions()`: replace every `GetPrivateProfileInt` / `GetPrivateProfileString` on `options.IniFileName` with the layered read; keep the first-run sample->INI copy
- [x] 2.2 `HTMLView/functions.cpp` `CSmallStringList::load_from_ini` / `load_sign_from_ini`: read through the layered helper and take the hardcoded default as a parameter
- [x] 2.3 `HTMLView/main.cpp` `InitProc()`: route the `[Extensions]`, `[Hotkeys]` and `[Renderer]` reads through the layered read
- [x] 2.4 `HTMLView/browserhost.cpp`: route the `[Hotkeys]` action, `ListerTitle` and `NewWindowCHM` / `NewWindowHTML` reads through the layered read
- [x] 2.5 Verify that `positions.ini` and `MarkdownView_Log.ini` still use the plain `GetPrivateProfile*` calls (they are not plugin configuration)
- [x] 2.6 Show the "file not found" warning only when neither `MarkdownView.ini` nor `MarkdownView.ini.sample` exists

## 3. Align the hardcoded defaults with the sample (audit table)

- [x] 3.1 Collect the mirrored defaults in `HTMLView/settings_defaults.h`
- [x] 3.2 Apply the audit table in `design.md`: `[options] ShowDirs`, `ShowStatusbar`, `ShowToolbar`, `ShowVideos`, `PlaySounds`, `AllowActiveX`, `NewWindowHTML`, `NewWindowCHM`
- [x] 3.3 Apply the audit table in `design.md`: `[Extensions] CHMExtensions`, `HTMLExtensions`, `MarkdownExtensions`, `DefaultSignatures`, `SignatureSkipSpaces`
- [x] 3.4 Apply the audit table in `design.md`: `[Hotkeys] TranslationHotkeys`, `TypingTranslationHotkeys`
- [x] 3.5 Apply the audit table in `design.md`: `[Renderer] HoedownArgs`, `UseSmartypants`, `CustomCSS`, `CustomCSSDark`
- [x] 3.6 Leave the commented-out `[Debug]` keys at their current hardcoded defaults

## 4. Tests

- [x] 4.1 Add `tests/ini_settings_test.cpp` covering the 2x2 matrix: sample only, user overlay (one overridden key, one inherited key, one absent everywhere), user only, neither file - asserting the exact string per layer
- [x] 4.2 Add the audit-table keys to the test so every setting resolves as documented, and compare them with the shipped `Build/MarkdownView.ini` (drift guard)
- [x] 4.3 Add `tests/run_tests.bat` that builds and runs the test for Win32 and x64
- [x] 4.4 Document the test command in `AGENTS.md`

## 5. Documentation

- [x] 5.1 Note in `Build/MarkdownView.ini` that it is the source of the shipped defaults
- [x] 5.2 Update `AGENTS.md` and `openspec/conventions.md` with the layered read

## 6. Verification

- [x] 6.1 Build Release for both Win32 and x64 and verify both builds succeed
- [x] 6.2 Run `tests/run_tests.bat` for both architectures
- [ ] 6.3 Manually load the plugin in Total Commander with (a) no INI and a read-only directory, (b) an INI that overrides one key, and (c) an INI missing a key that the sample provides
