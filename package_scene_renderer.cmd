cd /D "%~dp0" || exit /b

for %%I in ( ^
    freealut\build\src\Release\alut.dll ^
    VSRecastBuild\Detour\Release\Detour.dll ^
    VSRecastBuild\DebugUtils\Release\DebugUtils.dll ^
    VSRecastBuild\Recast\Release\Recast.dll ^
    glfw_vc2022\lib\glfw3.dll ^
    ) do copy %%I GVS\Bin\Release\
