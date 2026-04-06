#pragma once

struct android_app;
struct AndroidAppGuard;

class AndroidApp {
    friend AndroidAppGuard;
public:
    static android_app& App();
private:
    static void Init(android_app &app);
    static void Destroy();
    static android_app* app_;
};

struct AndroidAppGuard {
    explicit AndroidAppGuard(android_app& app);
    ~AndroidAppGuard();
};
