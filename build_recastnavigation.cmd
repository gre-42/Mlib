cd /D "%~dp0" || exit /b

IF "%CMAKE_BUILD_TYPE%"=="" set CMAKE_BUILD_TYPE=Release
set BUILD_DIR=VSRecastBuild
mkdir %BUILD_DIR%
cmake ^
    -S recastnavigation ^
    -B %BUILD_DIR% ^
    %CMAKE_OPTIONS% ^
    -DRECASTNAVIGATION_DEMO=OFF ^
    -DRECASTNAVIGATION_TESTS=OFF ^
    -DRECASTNAVIGATION_EXAMPLES=OFF ^
    -DBUILD_SHARED_LIBS=ON || exit /b
cmake ^
    --build %BUILD_DIR% ^
    --config %CMAKE_BUILD_TYPE% ^
    --verbose || exit /b
