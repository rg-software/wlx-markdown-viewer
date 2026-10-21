# Project conventions

Canonical conventions for `wlx-markdown-viewer`. `AGENTS.md` holds the operational build, test, and architecture details; this file is the OpenSpec-facing summary.

## Tech stack

- Total Commander WLX lister plugin, Windows 32/64-bit only.
- Renders HTML and Markdown via the Internet Explorer WebBrowser control; Markdown is converted with the vendored `hoedown` engine (`Hoedown/`).
- MSVC C++ (v143 toolset), `MultiByte` charset, static CRT (`MultiThreaded` / `MultiThreadedDebug`).
- Single solution: `MarkdownView.sln`.
- No vcpkg or external package manager.

## Vendored dependencies

- `boost/` (nowide, header-only)
- `ChmLib/`
- `HTMLView/TextEncodingDetect/`
- `Hoedown/`

## Architecture

- TC WLX exports (`ListLoad`, `ListLoadNext`, `ListSearchText`/`W`, `ListPrint`, `ListCloseWindow`) live in `HTMLView/main.cpp`, exported via `HTMLView/htmlview.def`.
- Markdown files are detected by extension from `MarkdownView.ini` (`[Extensions] MarkdownExtensions`); other supported types (HTML, CHM, signatures) go straight to the IE WebBrowser control.
- Markdown rendering pipeline (`browser_show_file` in `HTMLView/main.cpp`): read file with encoding detection (text-encoding-detect), run `hoedown_main`, optionally run `smartypants`, inject configured CSS from `Build/css/`, load result into the WebBrowser control via a stream.
- Plugin settings come from `MarkdownView.ini` (template in `Build/MarkdownView.ini`): `[Extensions]`, `[Renderer]` (HoedownArgs, UseSmartypants, CustomCSS/CustomCSSDark), hotkeys, toolbar/status bar options. They are read through a layered lookup (`HTMLView/ini_settings.*`): the user's `MarkdownView.ini`, then the shipped `MarkdownView.ini.sample` from the same directory, then the hardcoded default from `HTMLView/settings_defaults.h` (which mirrors the active values of `Build/MarkdownView.ini`). Lists are parsed with `CSmallStringList` (`HTMLView/functions.*`).
- Encoding detection: `TextEncodingDetect` plus `boost::nowide` for UTF-16 to UTF-8 conversion.

## Conventions

- Keep changes within the existing project layout; do not introduce new dependencies or package managers.
- Prefer editing C/C++ sources and the `Build/css/` sheets over restructuring.
- Conventional Commits (`feat:`, `fix:`, `docs:`, `refactor:`, `chore:`), subject under 72 characters, so the release pipeline can group them.
- `master` is the single source of truth; no separate platform branches.

## Verification

- Build Release for both Win32 and x64 (`msbuild MarkdownView.sln /t:Build /p:Configuration=Release;Platform=Win32|x64;UseEnv=true`).
- Configuration unit tests: `tests\run_tests.bat` (layered settings lookup, defaults vs. `Build/MarkdownView.ini`).
- Manual load in Total Commander.
- Hoedown rendering changes: `python Hoedown/test/runner.py` (requires `tidy`).