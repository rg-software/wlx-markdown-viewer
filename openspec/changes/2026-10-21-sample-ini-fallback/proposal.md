# Proposal

## Why

`MarkdownView.ini.sample` (shipped in the release archive as the template) is supposed to describe the plugin's default configuration, but the plugin does not read it. Every setting is read from the user's `MarkdownView.ini` only, and when that file cannot be created - for example when the plugin directory is read-only - the plugin silently falls back to hardcoded read-site defaults.

Those hardcoded defaults have drifted away from the sample:

- `[Extensions] MarkdownExtensions`, `HTMLExtensions`, `CHMExtensions`, `DefaultSignatures` default to an empty list, so a user without a usable INI gets **no file type detected at all** (nothing renders).
- `[Renderer] HoedownArgs`, `UseSmartypants`, `CustomCSS`, `CustomCSSDark` default to empty strings, so Markdown is rendered without arguments and without CSS.
- `[Hotkeys]` lists default to empty, so none of the plugin's own hotkeys work.
- `[options] ShowToolbar`, `ShowStatusbar`, `ShowDirs` default to 0 (hidden), while `ShowVideos`, `PlaySounds` default to 1 (enabled) and `AllowActiveX` to 0, contradicting the sample.

The result is a configuration that a user cannot reach by editing the sample, and that silently differs from the configuration every user with a writable directory gets (the first-run copy of the sample).

## What Changes

- Add a layered configuration read: **user `MarkdownView.ini` -> shipped `MarkdownView.ini.sample` (same directory) -> hardcoded default at the read site**. The first layer that contains the section/key wins; a key missing from both files still resolves to the hardcoded default.
- Route every plugin setting read through that helper (`HTMLView/functions.cpp`, `HTMLView/main.cpp`, `HTMLView/browserhost.cpp`). Call-site logic is otherwise unchanged.
- Align the hardcoded read-site defaults with the *active* values of the sample so that the sample and the code agree (documented audit table in `design.md`). Commented-out sample keys stay inactive and keep their current hardcoded defaults.
- Keep the existing first-run copy of the sample to `MarkdownView.ini`; it now complements the fallback layer instead of overlapping with it (creation covers writable directories, the fallback layer covers read-only ones).
- Add a unit test that covers the 2x2 matrix (sample only / user overlay / user only / neither file) and every key of the audit table.

## Capabilities

### New Capabilities
<!-- None -->

### Modified Capabilities
- `plugin-config`: extend configuration loading with the shipped sample as a fallback layer between the user's INI and the hardcoded defaults.

## Impact

- Affected code: new `HTMLView/ini_settings.h` / `HTMLView/ini_settings.cpp`, new `HTMLView/settings_defaults.h` (the mirrored shipped defaults), `HTMLView/functions.cpp` (`InitOptions`, `CSmallStringList::load_from_ini`, `CSmallStringList::load_sign_from_ini`), `HTMLView/main.cpp` (`InitProc`), `HTMLView/browserhost.cpp` (`ProcessHotkey`, `UpdateTitle`, `DISPID_NEWWINDOW`).
- Affected files: `HTMLView/MarkdownView.vcxproj` (+ `.filters`), new `tests/ini_settings_test.cpp`, `Build/MarkdownView.ini` (comment header only - its values stay authoritative), `AGENTS.md` / `openspec/conventions.md` (architecture notes).
- Packaging is unchanged: the archive still ships `MarkdownView.ini.sample` and no `MarkdownView.ini`.
- User-visible effect: users whose `MarkdownView.ini` lacks a key now inherit that key from the shipped sample instead of from a stale hardcoded default; users without any INI (read-only plugin directory) get the shipped configuration instead of a broken one.
