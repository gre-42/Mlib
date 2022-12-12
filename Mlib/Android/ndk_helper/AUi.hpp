#pragma once
#include <NDKHelper.h>
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
    static std::unique_ptr<std::istream> OpenFile(const std::string& filename);
    static std::vector<uint8_t> ReadFile(const std::string& filename);
    static bool PathExists(const std::string& path);
    static ndk_helper::DirectoryIterator ListDir(const std::string& dirname);
    static std::string GetExternalFilesDir();
    static void SetRequestedScreenOrientation(ScreenOrientation orientation);
private:
    static android_app& App();
    static android_app* app_;
};
