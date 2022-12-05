#pragma once
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
    static bool FileExists(const std::string& filename);
private:
    static android_app& App();
    static android_app* app_;
};
