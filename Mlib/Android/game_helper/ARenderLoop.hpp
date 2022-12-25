#pragma once
#include <functional>

struct android_app;
class AEngine;

class ARenderLoop {
public:
    ARenderLoop(
        android_app &app,
        AEngine &aengine,
        const char *monstartup_lib = nullptr);

    void render_loop(const std::function<bool()>& exit_loop = [](){return false;});
    bool destroy_requested() const;
private:
    android_app &app_;
    AEngine &aengine_;
};
