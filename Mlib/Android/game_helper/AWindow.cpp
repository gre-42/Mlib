#include "AWindow.hpp"
#include <Mlib/Android/ndk_helper/GLContext.h>
#include <dlfcn.h>

typedef int32_t (*PF_SET_FRAME_RATE)(ANativeWindow* window, float frameRate, int8_t compatibility);
static PF_SET_FRAME_RATE ANativeWindow_setFrameRate_dynamic = nullptr;

AWindow::AWindow(android_app& app)
: app_{app}
{}

AWindow::~AWindow() {
    ndk_helper::GLContext::GetInstance()->Invalidate();
}

void AWindow::make_current() const {
    ndk_helper::GLContext::GetInstance()->Resume(app_.window);
}

void AWindow::unmake_current() const {
    ndk_helper::GLContext::GetInstance()->Suspend();
}

// From: https://github.com/Rakashazi/emu-ex-plus-alpha/blob/master/imagine/src/base/android/AndroidWindow.cc
void AWindow::set_frame_rate_if_supported(float rate) const {
    if (AConfiguration_getSdkVersion(app_.config) < 30) {
        return;
    }
    if (app_.window == nullptr) {
        LOGE("ARenderLoop::set_frame_rate called on null window");
        std::abort();
    }
    if (ANativeWindow_setFrameRate_dynamic == nullptr)     {
        void* androidHandle = dlopen("libnativewindow.so", RTLD_NOW);
        if (androidHandle == nullptr) {
            LOGE("Could not load libnativewindow.so: %s", dlerror());
            std::abort();
        }
        ANativeWindow_setFrameRate_dynamic = (PF_SET_FRAME_RATE)
            dlsym(androidHandle, "ANativeWindow_setFrameRate");
        if (ANativeWindow_setFrameRate_dynamic == nullptr) {
            LOGE("Could not get ANativeWindow_setFrameRate: %s", dlerror());
            std::abort();
        }
    }
    if (ANativeWindow_setFrameRate_dynamic(
        app_.window,
        rate,
        ANATIVEWINDOW_FRAME_RATE_COMPATIBILITY_FIXED_SOURCE) != 0)
    {
        LOGE("error in ANativeWindow_setFrameRate() with window:%p rate:%.2f", app_.window, rate);
        std::abort();
    }
}
