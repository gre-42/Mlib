Push-Location $PSScriptRoot

Write-Host "Compile OpenAL Soft"
git clone https://github.com/kcat/openal-soft.git
cd openal-soft/build
cmake ..
cmake --build . --config Release
cd ..\..

Write-Host "Compile freealut"
git clone https://github.com/vancegroup/freealut
mkdir freealut/build
cd freealut/build
cmake `
    .. `
    -DBUILD_EXAMPLES=off `
    -DBUILD_TESTS=off  `
    "-DOPENAL_LIBRARY=..\..\openal-soft\build\Release\OpenAL32.lib" `
    "-DOPENAL_INCLUDE_DIR=..\..\openal-soft\include\AL"
cmake --build . --config Release

Pop-Location
