cd /D "%~dp0" || exit /b

IF "%CMAKE_BUILD_TYPE%"=="" set CMAKE_BUILD_TYPE=Release
set BUILD_DIR=%BUILD_PREFIX%VS
mkdir %BUILD_DIR%
cd %BUILD_DIR% || exit /b
set OPENALDIR=C:\Program Files (x86)\OpenAL 1.1 SDK
cmake .. %CMAKE_OPTIONS% || exit /b
cmake ^
    --build . ^
    --config %CMAKE_BUILD_TYPE% ^
    --verbose || exit /b
