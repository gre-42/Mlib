#pragma once
#include <Mlib/Os/Utf8_Path.hpp>
#include <Mlib/Os/ndk_helper/JNIHelper.h>
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
        const Utf8Path& filename,
        std::ios_base::openmode mode);
    static std::vector<uint8_t> ReadFile(const Utf8Path& filename);
    static bool PathExists(const Utf8Path& path);
    static ndk_helper::DirectoryIterator ListDir(const Utf8Path& dirname);
    static std::string GetFilesDir(ndk_helper::StorageType storage_type);
    static std::string GetFlavor();
    static void SetRequestedScreenOrientation(ScreenOrientation orientation);
    static void RequestReadExternalStoragePermission();
    static void SetThreadName(const std::string& name);
};
