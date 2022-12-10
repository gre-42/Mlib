#pragma once
#include <NDKHelper.h>
#include <iostream>
#include <memory>
#include <string>

struct android_app;

class AUi {
public:
    static void Init(android_app& app);
    static void ShowMessage(
        const std::string& title,
        const std::string& message);
    static std::unique_ptr<std::istream> OpenFile(const std::string& filename);
    static bool PathExists(const std::string& path);
    static ndk_helper::DirectoryIterator ListDir(const std::string& dirname);
private:
    static android_app& App();
    static android_app* app_;
};
