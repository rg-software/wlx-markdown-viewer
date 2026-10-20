# AGENTS.md

> **Repository hosting**: This project is on **GitHub** at `rg-software/wlx-markdown-viewer`. Issues and pull requests are read/written with the **`gh` CLI** (`gh issue`, `gh pr`). The remote `git` origin also points at GitHub.

Lister plugin (32/64-bit Windows) for Total Commander that renders HTML and Markdown files via the Internet Explorer WebBrowser control. Markdown is converted to HTML with the vendored `hoedown` engine (see `Hoedown/`). Based on HTMLView 1.2.6 source; also handles CHM and other IE-viewable content.

## Build

- Requires **MS Visual Studio 2022** (v143 toolset). No vcpkg, no external package manager.
- Vendored dependencies: `boost/` (nowide, header-only), `ChmLib/`, `HTMLView/TextEncodingDetect/`, and the `Hoedown/` source tree.
- From a VS 2022 Developer Command Prompt (or as `BuildMakeSetup.bat` does - `vcvarsall.bat x86|x64` then msbuild):
  - Win32 release: `msbuild MarkdownView.sln /t:Build /p:Configuration=Release;Platform=Win32;UseEnv=true`
  - x64 release: `msbuild MarkdownView.sln /t:Build /p:Configuration=Release;Platform=x64;UseEnv=true`
- The solution builds `hoedown-lib`, `hoedown`, `smartypants` first (project dependencies), then `MarkdownView`.
- Per-config outputs: `Build\MarkdownView.wlx` (Win32) and `Build\MarkdownView.wlx64` (x64) for Release; Debug x64 emits `Build\MarkdownViewD.wlx64`.
- `Hoedown/lib/*.lib` and `Hoedown/bin/*.exe` are build artifacts (gitignored).

## Release packaging

- `BuildMakeSetup.bat` builds Release for both platforms, assembles `ReleaseWLX/` (`css/`, `doc/`, ini, inf, wlx/wlx64), and zips it to `Release-YYYYMMDD.zip`.
- `ReleaseWLX/` and `*.zip` are gitignored build outputs - do not commit them.

## Tests

- Hoedown rendering tests: `python Hoedown/test/runner.py` (requires `tidy` installed and on PATH). Compares rendered Markdown output against expected HTML.
- No C++ unit-test suite and no linter; verification is a build of both platforms plus a manual load in Total Commander.

## Architecture

- Total Commander WLX exports (`ListLoad`, `ListLoadNext`, `ListSearchText`/`W`, `ListPrint`, `ListCloseWindow`, `ListGetDetectString`-adjacent) live in `HTMLView/main.cpp`, exported via `HTMLView/htmlview.def`.
- Markdown files are detected by extension (from `MarkdownView.ini` -> `[Extensions] MarkdownExtensions`); other supported types (HTML, CHM, signatures) are handed to the IE WebBrowser control directly.
- Markdown rendering pipeline (`browser_show_file` in `HTMLView/main.cpp`): read file (with encoding detection via text-encoding-detect), run `hoedown_main` to convert to HTML, optionally run `smartypants`, inject the configured CSS (from `Build/css/`), and load the result into the WebBrowser control via a stream.
- Plugin settings come from `MarkdownView.ini` (template shipped in `Build/MarkdownView.ini`): `[Extensions]`, `[Renderer]` (HoedownArgs, UseSmartypants, CustomCSS/CustomCSSDark), hotkeys, toolbar/status bar options. Parsed with `GetPrivateProfile*` and `CSmallStringList` (see `HTMLView/functions.*`).
- Encoding detection: `TextEncodingDetect` plus boost::nowide for UTF-16 -> UTF-8 conversion.

## Conventions

- MSVC C++ with `MultiByte` charset and static CRT (`MultiThreaded` / `MultiThreadedDebug`); toolset v143.
- Keep changes within the existing project layout; do not introduce new dependencies or package managers.
- Prefer editing the C/C++ sources and the `Build/css/` sheets over restructuring.
- Conventional Commits (`feat:`, `fix:`, `docs:`, `refactor:`, `chore:`) so the release pipeline can group them; keep the subject under 72 characters.

## Git

- `master` is the single source of truth; no separate platform branches.
- Do not run `git commit`, `git push`, `git reset`, or `git rebase` without explicit user confirmation.

## Agentic workflow

- The canonical architecture/conventions reference is `openspec/config.yaml` -> `context`. It is loaded into AI-assisted sessions automatically, so there is a single source of truth - do not duplicate it here.
- OpenSpec changes live under `openspec/changes/` (active) and `openspec/changes/archive/` (completed); main specs under `openspec/specs/`.