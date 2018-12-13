:: This script creates a release (setup) package using a prebuilt project.
@echo off

del /Q markdownviewer.zip
del Build\*.iobj
del Build\*.ipdb

powershell.exe -nologo -noprofile -command "& { Add-Type -A 'System.IO.Compression.FileSystem'; [IO.Compression.ZipFile]::CreateFromDirectory('Build', 'markdownviewer.zip'); }"
