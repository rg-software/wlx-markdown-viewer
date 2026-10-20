# Proposal

## Why

Installing or updating the plugin via Total Commander's auto-installer extracts the entire archive, including `MarkdownView.ini`, which overwrites the user's customized configuration. This causes settings loss on every update.

## What Changes

- Stop shipping `MarkdownView.ini` itself in the release archive; ship a template file instead (e.g. `MarkdownView.ini.sample`).
- On first run, if `MarkdownView.ini` is missing, copy the template to `MarkdownView.ini` before reading settings.
- Update packaging scripts/workflow to include the template instead of the live `.ini`.
- Update `InitOptions()` in `HTMLView/functions.cpp` to create the default INI from the template when it does not exist.

## Capabilities

### New Capabilities
- `plugin-config`: Loading and preserving plugin configuration (`MarkdownView.ini`), including first-run creation from a shipped template.

### Modified Capabilities
<!-- None -->

## Impact

- Affected code: `HTMLView/functions.cpp` (`InitOptions`), `BuildMakeSetup.bat`, `.github/workflows/release.yml`.
- Affected files: `Build/MarkdownView.ini` (becomes template), release archive contents.
- No breaking API changes; existing INI files remain untouched.