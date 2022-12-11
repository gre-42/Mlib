#include "AEngine.hpp"
#include <Mlib/Render/IRenderer.hpp>

//-------------------------------------------------------------------------
// Ctor
//-------------------------------------------------------------------------
AEngine::AEngine(Mlib::IRenderer& renderer)
: renderer_{renderer},
  initialized_resources_(false),
  has_focus_(false),
  app_(nullptr),
  sensor_manager_(nullptr),
  accelerometer_sensor_(nullptr),
  sensor_event_queue_(nullptr)
{
    gl_context_ = ndk_helper::GLContext::GetInstance();
}

//-------------------------------------------------------------------------
// Dtor
//-------------------------------------------------------------------------
AEngine::~AEngine() = default;

/**
 * Load resources
 */
void AEngine::LoadResources() {
    renderer_.load_resources();
}

/**
 * Unload resources
 */
void AEngine::UnloadResources() {
    renderer_.unload_resources();
}

/**
 * Initialize an EGL context for the current display.
 */
int AEngine::InitDisplay(android_app* app) {
    if (!initialized_resources_) {
        gl_context_->Init(app_->window);
        LoadResources();
        initialized_resources_ = true;
    } else if(app->window != gl_context_->GetANativeWindow()) {
        // Re-initialize ANativeWindow.
        // On some devices, ANativeWindow is re-created when the app is resumed
        assert(gl_context_->GetANativeWindow());
        UnloadResources();
        gl_context_->Invalidate();
        app_ = app;
        gl_context_->Init(app->window);
        LoadResources();
        initialized_resources_ = true;
    } else {
        // initialize OpenGL ES and EGL
        if (EGL_SUCCESS == gl_context_->Resume(app_->window)) {
            UnloadResources();
            LoadResources();
        } else {
            assert(0);
        }
    }

    ShowUI();

    // Initialize GL state.
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // Note that screen size might have been changed
    glViewport(0, 0, gl_context_->GetScreenWidth(),
               gl_context_->GetScreenHeight());
    renderer_.update_viewport();

    tap_camera_.SetFlip(1.f, -1.f, -1.f);
    tap_camera_.SetPinchTransformFactor(2.f, 2.f, 8.f);

    return 0;
}

void AEngine::DrawFrame(Mlib::RenderEvent event) {
    renderer_.render(
        event,
        gl_context_->GetScreenWidth(),
        gl_context_->GetScreenHeight());

    // Swap
    if (EGL_SUCCESS != gl_context_->Swap()) {
        UnloadResources();
        LoadResources();
    }
}

/**
 * Tear down the EGL context currently associated with the display.
 */
void AEngine::TermDisplay() { gl_context_->Suspend(); }

void AEngine::TrimMemory() {
    LOGI("Trimming memory");
    gl_context_->Invalidate();
}
/**
 * Process the next input event.
 */
int32_t AEngine::HandleInput(android_app* app, AInputEvent* event) {
    auto* eng = (AEngine*)app->userData;
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        for (size_t i = 0; i < AMotionEvent_getPointerCount(event); ++i) {
            if (AMotionEvent_getAction(event) == AMOTION_EVENT_ACTION_DOWN) {
                LOGI("Down %f, %d", AMotionEvent_getX(event, i), AMotionEvent_getPointerId(event, 0));
            }
            if (AMotionEvent_getAction(event) == AMOTION_EVENT_ACTION_UP) {
                LOGI("Up %f, %d", AMotionEvent_getX(event, i), AMotionEvent_getPointerId(event, 0));
            }
        }

        ndk_helper::GESTURE_STATE doubleTapState =
                eng->doubletap_detector_.Detect(event);
        ndk_helper::GESTURE_STATE dragState = eng->drag_detector_.Detect(event);
        ndk_helper::GESTURE_STATE pinchState = eng->pinch_detector_.Detect(event);

        // Double tap detector has a priority over other detectors
        if (doubleTapState == ndk_helper::GESTURE_STATE_ACTION) {
            // Detect double tap
            eng->tap_camera_.Reset(true);
        } else {
            // Handle drag state
            if (dragState & ndk_helper::GESTURE_STATE_START) {
                // Otherwise, start dragging
                ndk_helper::Vec2 v;
                eng->drag_detector_.GetPointer(v);
                eng->TransformPosition(v);
                eng->tap_camera_.BeginDrag(v);
            } else if (dragState & ndk_helper::GESTURE_STATE_MOVE) {
                ndk_helper::Vec2 v;
                eng->drag_detector_.GetPointer(v);
                eng->TransformPosition(v);
                eng->tap_camera_.Drag(v);
            } else if (dragState & ndk_helper::GESTURE_STATE_END) {
                eng->tap_camera_.EndDrag();
            }

            // Handle pinch state
            if (pinchState & ndk_helper::GESTURE_STATE_START) {
                // Start new pinch
                ndk_helper::Vec2 v1;
                ndk_helper::Vec2 v2;
                eng->pinch_detector_.GetPointers(v1, v2);
                eng->TransformPosition(v1);
                eng->TransformPosition(v2);
                eng->tap_camera_.BeginPinch(v1, v2);
            } else if (pinchState & ndk_helper::GESTURE_STATE_MOVE) {
                // Multi touch
                // Start new pinch
                ndk_helper::Vec2 v1;
                ndk_helper::Vec2 v2;
                eng->pinch_detector_.GetPointers(v1, v2);
                eng->TransformPosition(v1);
                eng->TransformPosition(v2);
                eng->tap_camera_.Pinch(v1, v2);
            }
        }
        return 1;
    }
    return 0;
}

/**
 * Process the next main command.
 */
void AEngine::HandleCmd(struct android_app* app, int32_t cmd) {
    auto* eng = (AEngine*)app->userData;
    switch (cmd) {
        case APP_CMD_SAVE_STATE:
            break;
        case APP_CMD_INIT_WINDOW:
            // The window is being shown, get it ready.
            if (app->window != nullptr) {
                eng->InitDisplay(app);
                eng->has_focus_ = true;
                eng->DrawFrame(Mlib::RenderEvent::INIT_WINDOW);
            }
            break;
        case APP_CMD_TERM_WINDOW:
            // The window is being hidden or closed, clean it up.
            eng->TermDisplay();
            eng->has_focus_ = false;
            break;
        case APP_CMD_STOP:
            break;
        case APP_CMD_GAINED_FOCUS:
            eng->ResumeSensors();
            // Start animation
            eng->has_focus_ = true;
            break;
        case APP_CMD_LOST_FOCUS:
            eng->SuspendSensors();
            // Also stop animating.
            eng->has_focus_ = false;
            eng->DrawFrame(Mlib::RenderEvent::LOST_FOCUS);
            break;
        case APP_CMD_LOW_MEMORY:
            // Free up GL resources
            eng->TrimMemory();
            break;
        default:
            // Do nothing
            ;
    }
}

//-------------------------------------------------------------------------
// Sensor handlers
//-------------------------------------------------------------------------
void AEngine::InitSensors() {
    sensor_manager_ = ndk_helper::AcquireASensorManagerInstance(app_);
    accelerometer_sensor_ = ASensorManager_getDefaultSensor(
            sensor_manager_, ASENSOR_TYPE_ACCELEROMETER);
    sensor_event_queue_ = ASensorManager_createEventQueue(
            sensor_manager_, app_->looper, LOOPER_ID_USER, nullptr, nullptr);
}

void AEngine::ProcessSensors(int32_t id) {
    // If a sensor has data, process it now.
    if (id == LOOPER_ID_USER) {
        if (accelerometer_sensor_ != nullptr) {
            ASensorEvent event;
            while (ASensorEventQueue_getEvents(sensor_event_queue_, &event, 1) > 0) {
            }
        }
    }
}

void AEngine::ResumeSensors() {
    // When our app gains focus, we start monitoring the accelerometer.
    if (accelerometer_sensor_ != nullptr) {
        ASensorEventQueue_enableSensor(sensor_event_queue_, accelerometer_sensor_);
        // We'd like to get 60 events per second (in us).
        ASensorEventQueue_setEventRate(sensor_event_queue_, accelerometer_sensor_,
                                       (1000L / 60) * 1000);
    }
}

void AEngine::SuspendSensors() {
    // When our app loses focus, we stop monitoring the accelerometer.
    // This is to avoid consuming battery while not being used.
    if (accelerometer_sensor_ != nullptr) {
        ASensorEventQueue_disableSensor(sensor_event_queue_, accelerometer_sensor_);
    }
}

//-------------------------------------------------------------------------
// Misc
//-------------------------------------------------------------------------
void AEngine::SetState(android_app* state) {
    app_ = state;
    doubletap_detector_.SetConfiguration(app_->config);
    drag_detector_.SetConfiguration(app_->config);
    pinch_detector_.SetConfiguration(app_->config);
}

bool AEngine::IsReady() const {
    if (has_focus_) return true;

    return false;
}

void AEngine::TransformPosition(ndk_helper::Vec2& vec) {
    vec = ndk_helper::Vec2(2.0f, 2.0f) * vec /
          ndk_helper::Vec2((float)gl_context_->GetScreenWidth(),
                           (float)gl_context_->GetScreenHeight()) -
          ndk_helper::Vec2(1.f, 1.f);
}

void AEngine::ShowUI() {
    JNIEnv* jni;
    app_->activity->vm->AttachCurrentThread(&jni, nullptr);

    // Default class retrieval
    jclass clazz = jni->GetObjectClass(app_->activity->clazz);
    jmethodID methodID = jni->GetMethodID(clazz, "showUI", "()V");
    jni->CallVoidMethod(app_->activity->clazz, methodID);

    app_->activity->vm->DetachCurrentThread();
}
