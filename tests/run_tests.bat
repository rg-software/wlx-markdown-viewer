@echo off
:: Builds and runs the configuration unit tests (tests\ini_settings_test.cpp)
:: for Win32 and x64. Run from a VS 2022 Developer Command Prompt or directly:
::   tests\run_tests.bat
setlocal
cd /d "%~dp0.."

if "%VCINSTALLDIR%"=="" set "VCINSTALLDIR=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\"
set "VCVARS=%VCINSTALLDIR%Auxiliary\Build\vcvarsall.bat"

if not exist tests\bin mkdir tests\bin

echo === Win32 ===
setlocal
call "%VCVARS%" x86 >nul
if not exist tests\bin\obj32 mkdir tests\bin\obj32
cl /nologo /EHsc /W3 /D_CRT_SECURE_NO_WARNINGS /Fe:tests\bin\ini_settings_test32.exe /Fo:tests\bin\obj32\ tests\ini_settings_test.cpp HTMLView\ini_settings.cpp
if errorlevel 1 (endlocal & exit /b 1)
tests\bin\ini_settings_test32.exe Build\MarkdownView.ini
if errorlevel 1 (endlocal & exit /b 1)
endlocal

echo === x64 ===
setlocal
call "%VCVARS%" x64 >nul
if not exist tests\bin\obj64 mkdir tests\bin\obj64
cl /nologo /EHsc /W3 /D_CRT_SECURE_NO_WARNINGS /Fe:tests\bin\ini_settings_test64.exe /Fo:tests\bin\obj64\ tests\ini_settings_test.cpp HTMLView\ini_settings.cpp
if errorlevel 1 (endlocal & exit /b 1)
tests\bin\ini_settings_test64.exe Build\MarkdownView.ini
if errorlevel 1 (endlocal & exit /b 1)
endlocal

echo All tests passed.
exit /b 0
