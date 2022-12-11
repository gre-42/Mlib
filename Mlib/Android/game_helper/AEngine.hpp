#pragma once
#include <NDKHelper.h>

namespace Mlib {
    class IRenderer;
    enum class RenderEvent;
}

// From: https://github.com/android/ndk-samples/blob/main/teapots/classic-teapot/src/main/cpp/TeapotNativeActivity.cpp

//-------------------------------------------------------------------------
// Shared state for our app.
//-------------------------------------------------------------------------
struct android_app;
class AEngine {
    Mlib::IRenderer& renderer_;

    ndk_helper::GLContext* gl_context_;

    bool initialized_resources_;
    bool has_focus_;

    ndk_helper::DoubletapDetector doubletap_detector_;
    ndk_helper::PinchDetector pinch_detector_;
    ndk_helper::DragDetector drag_detector_;

    ndk_helper::TapCamera tap_camera_;

    android_app* app_;

    ASensorManager* sensor_manager_;
    const ASensor* accelerometer_sensor_;
    ASensorEventQueue* sensor_event_queue_;

    void ShowUI();
    void TransformPosition(ndk_helper::Vec2& vec);

public:
    static void HandleCmd(struct android_app* app, int32_t cmd);
    static int32_t HandleInput(android_app* app, AInputEvent* event);

    explicit AEngine(Mlib::IRenderer& renderer);
    ~AEngine();
    void SetState(android_app* app);
    int InitDisplay(android_app* app);
    void LoadResources();
    void UnloadResources();
    void DrawFrame(Mlib::RenderEvent event);
    void TermDisplay();
    void TrimMemory();
    bool IsReady() const;

    void InitSensors();
    void ProcessSensors(int32_t id);
    void SuspendSensors();
    void ResumeSensors();
};
