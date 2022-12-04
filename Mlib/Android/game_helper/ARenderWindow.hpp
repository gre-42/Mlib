#pragma once
#include <functional>

struct android_app;
class AEngine;

class ARenderWindow {
public:
    ARenderWindow(
        android_app &app,
        AEngine &aengine,
        const char *monstartup_lib = nullptr);

    void render_loop(const std::function<bool()>& exit_loop = [](){return false;});
    bool window_should_close() const;
private:
    android_app &app_;
    AEngine &aengine_;
};
