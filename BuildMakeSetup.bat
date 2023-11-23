:: This script creates a release (setup) package using a prebuilt project.
@echo off
setlocal

call "%VCINSTALLDIR%\Auxiliary\Build\vcvarsall.bat" x86
msbuild MarkdownView.sln /t:Build /p:Configuration=Release;Platform=Win32;UseEnv=true

call "%VCINSTALLDIR%\Auxiliary\Build\vcvarsall.bat" x64
msbuild MarkdownView.sln /t:Build /p:Configuration=Release;Platform=x64;UseEnv=true


rmdir /S /Q ReleaseWLX
mkdir ReleaseWLX
mkdir ReleaseWLX\css

copy Readme.md ReleaseWLX\
copy hoedown.html ReleaseWLX\
copy Build\*.ini ReleaseWLX\
copy Build\*.wlx? ReleaseWLX\
copy Build\*.inf ReleaseWLX\
copy Build\*.md ReleaseWLX\
copy Build\css\*.* ReleaseWLX\css\

del /Q Release*.zip
powershell.exe -nologo -noprofile -command "& { Add-Type -A 'System.IO.Compression.FileSystem'; [IO.Compression.ZipFile]::CreateFromDirectory('ReleaseWLX', 'Release-' + (get-date -Format yyyyMMdd) +'.zip'); }"
