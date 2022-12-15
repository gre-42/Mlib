#pragma once
#include <NDKHelper.h>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>

struct android_app;

// From: https://developer.android.com/reference/android/content/pm/ActivityInfo#SCREEN_ORIENTATION_UNSPECIFIED
enum class ScreenOrientation {
    SCREEN_ORIENTATION_LANDSCAPE = 0,
    SCREEN_ORIENTATION_PORTRAIT = 1
};

class AUi {
public:
    static void Init(android_app& app);
    static void ShowMessage(
        const std::string& title,
        const std::string& message);
    static std::unique_ptr<std::istream> OpenFile(
        const std::filesystem::path& filename,
        std::ios_base::openmode mode);
    static std::vector<uint8_t> ReadFile(const std::filesystem::path& filename);
    static bool PathExists(const std::filesystem::path& path);
    static ndk_helper::DirectoryIterator ListDir(const std::filesystem::path& dirname);
    static std::string GetExternalFilesDir();
    static void SetRequestedScreenOrientation(ScreenOrientation orientation);
    static void RequestReadExternalStoragePermission();
private:
    static android_app& App();
    static android_app* app_;
};
