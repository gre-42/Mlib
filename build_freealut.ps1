Push-Location $PSScriptRoot

# 1. Install Chocolatey
Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))

# Install CMake and OpenAL SDK
choco install -y cmake --installargs ADD_CMAKE_TO_PATH=System
choco install -y openalsdk

# Invoke-WebRequest https://www.openal.org/downloads/OpenAL11CoreSDK.zip -OutFile OpenAL11CoreSDK.zip
# Expand-Archive OpenAL11CoreSDK.zip -DestinationPath .\
# .\OpenAL11CoreSDK.exe

# Compile freealut
git clone https://github.com/vancegroup/freealut
mkdir freealut/build
cd freealut/build
$env:OPENALDIR="C:\Program Files (x86)\OpenAL 1.1 SDK"
& "C:\Program Files\CMake\bin\cmake" ..
& "C:\Program Files\CMake\bin\cmake" --build . --config Release

Pop-Location
