cd /D "%~dp0" || exit /b

IF "%CMAKE_BUILD_TYPE%"=="" set CMAKE_BUILD_TYPE=Release
set BUILD_DIR=%BUILD_PREFIX%VSRecastBuild
mkdir %BUILD_DIR%
cd %BUILD_DIR% || exit /b
cmake ../recastnavigation ^
    %CMAKE_OPTIONS% ^
    -DRECASTNAVIGATION_DEMO=OFF ^
    -DRECASTNAVIGATION_TESTS=OFF ^
    -DRECASTNAVIGATION_EXAMPLES=OFF ^
    -DBUILD_SHARED_LIBS=ON || exit /b
cmake ^
    --build . ^
    --config %CMAKE_BUILD_TYPE% ^
    --verbose || exit /b
