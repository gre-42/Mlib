cd /D "%~dp0" || exit /b

set BUILD_PREFIX=G
set CMAKE_OPTIONS=-DBUILD_TRIANGLE=OFF -DBUILD_CV=OFF -DBUILD_SFM=OFF -DBUILD_OPENCV=OFF -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake
build.cmd || exit /b
