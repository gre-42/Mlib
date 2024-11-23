cd /D "%~dp0" || exit /b

IF "%CMAKE_BUILD_TYPE%"=="" set CMAKE_BUILD_TYPE=Release
set BUILD_DIR=VSZlibBuild
mkdir %BUILD_DIR%
cd %BUILD_DIR% || exit /b
cmake ../zlib ^
    %CMAKE_OPTIONS% ^
    -DZLIB_BUILD_EXAMPLES=OFF || exit /b
cmake ^
    --build . ^
    --config %CMAKE_BUILD_TYPE% ^
    --verbose || exit /b
