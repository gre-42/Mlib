cd /D "%~dp0" || exit /b

IF "%CMAKE_BUILD_TYPE%"=="" set CMAKE_BUILD_TYPE=Release
set BUILD_DIR=%BUILD_PREFIX%VS
mkdir %BUILD_DIR%
REM set OPENALDIR=C:\Program Files (x86)\OpenAL 1.1 SDK
cmake ^
    -S . ^
    -B %BUILD_DIR% ^
    %CMAKE_OPTIONS% || exit /b
cmake ^
    --build %BUILD_DIR% ^
    --config %CMAKE_BUILD_TYPE% ^
    --verbose || exit /b
