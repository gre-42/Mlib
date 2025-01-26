cd /D "%~dp0" || exit /b

PowerShell -ExecutionPolicy Bypass .\package_scene_renderer.ps1 || exit /b
