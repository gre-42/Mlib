cd /D "%~dp0" || exit /b

IF "%CMAKE_BUILD_TYPE%"=="" set CMAKE_BUILD_TYPE=Release

for %%I in ( ^
    openal-soft\build\Release\OpenAL32.dll ^
    freealut\build\src\Release\alut.dll ^
    VSRecastBuild\Detour\%CMAKE_BUILD_TYPE%\Detour.dll ^
    VSRecastBuild\DebugUtils\%CMAKE_BUILD_TYPE%\DebugUtils.dll ^
    VSRecastBuild\Recast\%CMAKE_BUILD_TYPE%\Recast.dll ^
    glfw_vc2022\lib\glfw3.dll ^
    ) do copy %%I GVS\Bin\%CMAKE_BUILD_TYPE%\ || exit /b
