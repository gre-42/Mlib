#pragma once
#include <Mlib/Android/ndk_helper/JNIHelper.h>
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
    static void ShowMessage(
        const std::string& title,
        const std::string& message);
    static std::unique_ptr<std::istream> OpenFile(
        const std::filesystem::path& filename,
        std::ios_base::openmode mode);
    static std::vector<uint8_t> ReadFile(const std::filesystem::path& filename);
    static bool PathExists(const std::filesystem::path& path);
    static ndk_helper::DirectoryIterator ListDir(const std::filesystem::path& dirname);
    static std::string GetFilesDir(ndk_helper::StorageType storage_type);
    static std::string GetFlavor();
    static void SetRequestedScreenOrientation(ScreenOrientation orientation);
    static void RequestReadExternalStoragePermission();
    static void SetThreadName(const std::string& name);
};
