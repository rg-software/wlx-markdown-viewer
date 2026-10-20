# Tasks

## 1. Runtime default creation

- [x] 1.1 Update `InitOptions()` in `HTMLView/functions.cpp` to copy `MarkdownView.ini.sample` to `MarkdownView.ini` when the `.ini` is missing, before reading settings; verify by deleting the `.ini` and confirming a default is created on next run
- [x] 1.2 Keep the existing warning when neither `.ini` nor `.sample` exists; verify by temporarily renaming both files and confirming the warning still appears

## 2. Packaging

- [x] 2.1 Update `BuildMakeSetup.bat` to ship `MarkdownView.ini.sample` instead of `MarkdownView.ini`; verify by running the script and checking `ReleaseWLX` contains the sample and no `.ini`
- [x] 2.2 Update `.github/workflows/release.yml` packaging step to copy `Build\MarkdownView.ini.sample` instead of `Build\*.ini`; verify the workflow YAML still parses and the packaged artifact contains the sample

## 3. Verification

- [x] 3.1 Build Release for both Win32 and x64 and verify both builds succeed
- [ ] 3.2 Manually install over an existing plugin directory and confirm the existing `MarkdownView.ini` is preserved