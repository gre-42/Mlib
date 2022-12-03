#include "ARenderWindow.hpp"
#include <Mlib/Android/game_helper/AEngine.hpp>
#include <Mlib/Render/IRenderer.hpp>

#define HELPER_CLASS_NAME \
  "com/hallo2hallo/helper/NDKHelper"  // Class name of helper function

ARenderWindow::ARenderWindow(
    android_app& app,
    AEngine& aengine,
    const char* monstartup_lib)
: app_{app},
  aengine_{aengine}
{
    aengine.SetState(&app);

    // Init helper functions
    ndk_helper::JNIHelper::Init(app.activity, HELPER_CLASS_NAME);

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

void ARenderWindow::render_loop() {
    // loop waiting for stuff to do.
    while (true) {
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
