@echo off
setlocal

@REM Clone submodules
call git submodule update --init --recursive

@REM Set up the version of Premake to download
set "premakeVersion=5.0.0-beta2"

@REM Create a directory to store the downloaded files
set "premakeBinDir=vendor\premake\bin"

@REM Premake zip file path
set "premakeZip=%premakeBinDir%\premake-%premakeVersion%-windows.zip"

@REM Premake bin path
set "premakeBin=%premakeBinDir%\premake5.exe"

@REM Premake LICENSE file path
set "premakeLicense=%premakeBinDir%\LICENSE.txt"

@REM Set the URLs to download
set "premakeUrl=https://github.com/premake/premake-core/releases/download/v%premakeVersion%/premake-%premakeVersion%-windows.zip"
set "licenseUrl=https://raw.githubusercontent.com/premake/premake-core/master/LICENSE.txt"

if not exist %premakeBin% (
    echo Continue only if you wish to download premake-v%premakeVersion% and its LICENSE at %premakeBinDir%
    PAUSE

    @REM Create binary dir for premake
    rd /s /q "%premakeBinDir%" 2>nul
    mkdir "%premakeBinDir%"

    @REM Download Premake
    curl -L -o "%premakeZip%" "%premakeUrl%"

    @REM Download LICENSE.txt
    curl -L -o "%premakeLicense%" "%licenseUrl%"

    @REM Unzip premake & delete premake zip file
    powershell -Command "Expand-Archive '%premakeZip%' -DestinationPath %premakeBinDir% -Force; Remove-Item -Path '%premakeZip%' -Force"
)

if not exist %premakeBin% (
    echo ERROR: Need premake for generating vs2022 files >&2
) else (
    @REM Generate VS2022 files
    call %premakeBin% vs2022
)

PAUSE