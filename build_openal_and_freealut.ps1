Push-Location $PSScriptRoot

# 1. Install Chocolatey
# Chocolatey is already installed, skipping
# Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))

Write-Host "Install CMake, OpenAL SDK and OpenAL"
choco install -y cmake --installargs ADD_CMAKE_TO_PATH=System
choco install -y openalsdk
choco install -y openal

# Invoke-WebRequest https://www.openal.org/downloads/OpenAL11CoreSDK.zip -OutFile OpenAL11CoreSDK.zip
# Expand-Archive OpenAL11CoreSDK.zip -DestinationPath .\
# .\OpenAL11CoreSDK.exe

Write-Host "Install freealut"
git clone https://github.com/vancegroup/freealut
mkdir freealut/build
cd freealut/build
$env:OPENALDIR="C:\Program Files (x86)\OpenAL 1.1 SDK"
cmake ..
cmake --build . --config Release

Pop-Location
