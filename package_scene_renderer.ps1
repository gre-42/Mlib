Push-Location $PSScriptRoot

if (Test-Path env:BUILD_PREFIX) {
    $BUILD_PREFIX = $env:BUILD_PREFIX
} else {
    $BUILD_PREFIX = ''
}

if (Test-Path env:CMAKE_BUILD_TYPE) {
    $CMAKE_BUILD_TYPE = $env:CMAKE_BUILD_TYPE
} else {
    $CMAKE_BUILD_TYPE = 'Release'
}

$openssl_path = Get-ItemPropertyValue -Path "HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\OpenSSL (64-bit)_is1" "Inno Setup: App Path"

Write-Host "Package scene renderer"
Write-Host "Build prefix: $BUILD_PREFIX"
Write-Host "CMAKE_BUILD_TYPE: $CMAKE_BUILD_TYPE"
Write-Host "OpenSSL path: $openssl_path"

$files = "openal-soft\build\Release\OpenAL32.dll",
         "freealut\build\src\Release\alut.dll",
         "$openssl_path\libssl-3-x64.dll",
         "$openssl_path\libcrypto-3-x64.dll",
         "VSRecastBuild\Detour\$CMAKE_BUILD_TYPE\Detour.dll",
         "VSRecastBuild\DebugUtils\$CMAKE_BUILD_TYPE\DebugUtils.dll",
         "VSRecastBuild\Recast\$CMAKE_BUILD_TYPE\Recast.dll",
         "VSZlibBuild\Release\zlib.dll",
         "glfw_vc2022\lib\glfw3.dll"

foreach ($file in $files) {
    $dest = "${BUILD_PREFIX}GVS\Bin\$CMAKE_BUILD_TYPE\"
    Write-Host "Copy $file -> $dest"
    Copy-Item $file -Destination $dest
}

Pop-Location
