#pragma once
#include <string>

struct android_app;

class AUi {
public:
    explicit AUi(android_app& app);
    void ShowMessage(
        const std::string& title,
        const std::string& message) const;
private:
    android_app& app_;
};
