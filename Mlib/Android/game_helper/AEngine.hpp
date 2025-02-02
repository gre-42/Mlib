#pragma once
#include <Mlib/Android/ndk_helper/NDKHelper.h>

namespace Mlib {
    class IRenderer;
    enum class RenderEvent;
    class ButtonStates;
    struct LayoutConstraintParameters;
}

// From: https://github.com/android/ndk-samples/blob/main/teapots/classic-teapot/src/main/cpp/TeapotNativeActivity.cpp

//-------------------------------------------------------------------------
// Shared state for our app.
//-------------------------------------------------------------------------
struct android_app;
class AEngine {
    Mlib::IRenderer& renderer_;
    Mlib::ButtonStates& buttons_states_;

    ndk_helper::GLContext* gl_context_;

    bool initialized_resources_;
    bool has_focus_;
    float xdpi_;
    float ydpi_;

    ndk_helper::DoubletapDetector doubletap_detector_;
    ndk_helper::PinchDetector pinch_detector_;
    ndk_helper::DragDetector drag_detector_;

    ndk_helper::TapCamera tap_camera_;

    android_app* app_;

    ASensorManager* sensor_manager_;
    const ASensor* accelerometer_sensor_;
    ASensorEventQueue* sensor_event_queue_;

    void TransformPosition(ndk_helper::Vec2& vec);
    void UpdateDpi();

public:
    static void HandleCmd(struct android_app* app, int32_t cmd);
    static int32_t HandleInput(android_app* app, AInputEvent* event);

    explicit AEngine(
        Mlib::IRenderer& renderer,
        Mlib::ButtonStates& buttons_states);
    ~AEngine();
    void SetState(android_app* app);
    int InitDisplay(android_app* app);
    void LoadResources();
    void UnloadResources();
    void DrawFrame(Mlib::RenderEvent event);
    void SuspendContext();
    bool ContextIsSuspended() const;
    void InvalidateContext();
    bool IsReady() const;

    void InitSensors();
    void ProcessSensors(int32_t id);
    void SuspendSensors();
    void ResumeSensors();

    Mlib::LayoutConstraintParameters LayoutParametersX() const;
    Mlib::LayoutConstraintParameters LayoutParametersY() const;
};
