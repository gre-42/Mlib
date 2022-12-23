#include "ARenderWindow.hpp"
#include <Mlib/Android/game_helper/AEngine.hpp>
#include <Mlib/Render/IRenderer.hpp>
#include <dlfcn.h>

enum ANativeWindow_FrameRateCompatibility {
    ANATIVEWINDOW_FRAME_RATE_COMPATIBILITY_DEFAULT = 0,
    ANATIVEWINDOW_FRAME_RATE_COMPATIBILITY_FIXED_SOURCE = 1
};
typedef int32_t (*PF_SET_FRAME_RATE)(ANativeWindow* window, float frameRate, int8_t compatibility);
static PF_SET_FRAME_RATE ANativeWindow_setFrameRate = nullptr;

ARenderWindow::ARenderWindow(
    android_app& app,
    AEngine& aengine,
    const char* monstartup_lib)
: app_{app},
  aengine_{aengine}
{
    aengine.SetState(&app);

    app.userData = &aengine;
    app.onAppCmd = AEngine::HandleCmd;
    app.onInputEvent = AEngine::HandleInput;

    if (monstartup_lib != nullptr) {
#ifdef USE_NDK_PROFILER
        monstartup(monstartup_lib);
#else
        throw std::runtime_error("Library built without support for android-ndk-profiler");
#endif
    }

    // Prepare to monitor accelerometer
    aengine.InitSensors();
}

void ARenderWindow::render_loop(const std::function<bool()>& exit_loop) {
    // loop waiting for stuff to do.
    while (!exit_loop()) {
        // Read all pending events.
        int id;
        int events;
        android_poll_source* source;

        // If not animating, we will block forever waiting for events.
        // If animating, we loop until all events are read, then continue
        // to draw the next frame of animation.
        while ((id = ALooper_pollAll(aengine_.IsReady() ? 0 : -1, nullptr, &events,
                                     (void**)&source)) >= 0) {
            // Process this event.
            if (source != nullptr) source->process(&app_, source);

            aengine_.ProcessSensors(id);

            // Check if we are exiting.
            if (app_.destroyRequested != 0) {
                aengine_.TermDisplay();
                return;
            }
        }

        if (aengine_.IsReady()) {
            // Drawing is throttled to the screen update rate, so there
            // is no need to do timing here.
            aengine_.DrawFrame(Mlib::RenderEvent::LOOP);
        }
    }
}

bool ARenderWindow::window_should_close() const {
    return app_.destroyRequested;
}

// From: https://github.com/Rakashazi/emu-ex-plus-alpha/blob/master/imagine/src/base/android/AndroidWindow.cc
void ARenderWindow::set_frame_rate_if_supported(float rate) {
    if (AConfiguration_getSdkVersion(app_.config) < 30) {
        return;
    }
    if (app_.window == nullptr) {
        LOGE("ARenderWindow::set_frame_rate called on null window");
        std::abort();
    }
    if (ANativeWindow_setFrameRate == nullptr)     {
        void* androidHandle = dlopen("libnativewindow.so", RTLD_NOW);
        if (androidHandle == nullptr) {
            LOGE("Could not load libnativewindow.so: %s", dlerror());
            std::abort();
        }
        ANativeWindow_setFrameRate = (PF_SET_FRAME_RATE)
            dlsym(androidHandle, "ANativeWindow_setFrameRate");
        if (ANativeWindow_setFrameRate == nullptr) {
            LOGE("Could not get ANativeWindow_setFrameRate: %s", dlerror());
            std::abort();
        }
    }
    if (ANativeWindow_setFrameRate(
        app_.window,
        rate,
        ANATIVEWINDOW_FRAME_RATE_COMPATIBILITY_FIXED_SOURCE) != 0)
    {
        LOGE("error in ANativeWindow_setFrameRate() with window:%p rate:%.2f", app_.window, rate);
        std::abort();
    }
}
