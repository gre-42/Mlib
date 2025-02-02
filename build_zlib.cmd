cd /D "%~dp0" || exit /b

IF "%CMAKE_BUILD_TYPE%"=="" set CMAKE_BUILD_TYPE=Release
set BUILD_DIR=VSZlibBuild
mkdir %BUILD_DIR%
cmake ^
    -S zlib ^
    -B %BUILD_DIR% ^
    %CMAKE_OPTIONS% ^
    -DZLIB_BUILD_EXAMPLES=OFF || exit /b
cmake ^
    --build %BUILD_DIR% ^
    --config %CMAKE_BUILD_TYPE% ^
    --verbose || exit /b
copy %BUILD_DIR%\zconf.h zlib\
