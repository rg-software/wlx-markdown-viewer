:: This script creates a release (setup) package using a prebuilt project.
@echo off

rmdir /S /Q ReleaseWLX
mkdir ReleaseWLX
mkdir ReleaseWLX\css

copy Build\*.ini ReleaseWLX\
copy Build\*.wlx? ReleaseWLX\
copy Build\*.inf ReleaseWLX\
copy Build\*.md ReleaseWLX\
copy Build\css\*.* ReleaseWLX\css\

del /Q markdownviewer.zip

powershell.exe -nologo -noprofile -command "& { Add-Type -A 'System.IO.Compression.FileSystem'; [IO.Compression.ZipFile]::CreateFromDirectory('ReleaseWLX', 'markdownviewer.zip'); }"
echo Resulting file markdownviewer.zip created!