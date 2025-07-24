#include "ARenderLoop.hpp"
#include <Mlib/Android/game_helper/AEngine.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Render/IRenderer.hpp>

ARenderLoop::ARenderLoop(
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
        Mlib::verbose_abort("Library built without support for android-ndk-profiler");
#endif
    }

    // Prepare to monitor accelerometer
    aengine.InitSensors();
}

void ARenderLoop::render_loop(const std::function<bool()>& exit_loop) {
    // loop waiting for stuff to do.
    while (!exit_loop()) {
        // Read all pending events.
        int id;
        int events;
        android_poll_source* source;

        // If not animating, we will block forever waiting for events.
        // If animating, we loop until all events are read, then continue
        // to draw the next frame of animation.
        while ((id = ALooper_pollOnce(aengine_.IsReady() ? 0 : -1, nullptr, &events,
                                     (void**)&source)) >= 0) {
            // Inspired by: https://stackoverflow.com/a/34961306/2292832
            if (id == ALOOPER_POLL_CALLBACK) {
                continue;
            }
            // Process this event.
            if (source != nullptr) {
                source->process(&app_, source);
            }
            aengine_.ProcessSensors(id);
            // Check if we are exiting.
            if (app_.destroyRequested != 0) {
                aengine_.SuspendContext();
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

bool ARenderLoop::destroy_requested() const {
    return app_.destroyRequested != 0;
}
