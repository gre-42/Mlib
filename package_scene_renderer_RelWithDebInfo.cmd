cd /D "%~dp0" || exit /b

set CMAKE_BUILD_TYPE=RelWithDebInfo
package_scene_renderer.cmd || exit /b
