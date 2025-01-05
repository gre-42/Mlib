#include "AEngine.hpp"
#include <Mlib/Android/ndk_helper/AndroidApp.hpp>
#include <Mlib/Android/ndk_helper/JNIHelper.h>
#include <Mlib/Layout/Layout_Constraint_Parameters.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Render/IRenderer.hpp>
#include <Mlib/Render/Ui/Button_States.hpp>

//-------------------------------------------------------------------------
// Ctor
//-------------------------------------------------------------------------
AEngine::AEngine(
    Mlib::IRenderer& renderer,
    Mlib::ButtonStates& buttons_states)
    : renderer_{ renderer }
    , buttons_states_{ buttons_states }
    , initialized_resources_{ false }
    , has_focus_{ false }
    , xdpi_{ 0 }
    , ydpi_{ 0 }
    , app_{ nullptr }
    , sensor_manager_{ nullptr }
    , accelerometer_sensor_{ nullptr }
    , sensor_event_queue_{ nullptr }
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
    if (app->window == nullptr) {
        Mlib::verbose_abort("AEngine::InitDisplay: window is null");
    }
    if (!initialized_resources_) {
        gl_context_->Init(app_->window);
        UpdateDpi();
        LoadResources();
        initialized_resources_ = true;
    } else if (app->window != gl_context_->GetANativeWindow()) {
        // Re-initialize ANativeWindow.
        // On some devices, ANativeWindow is re-created when the app is resumed
        // if (gl_context_->GetANativeWindow() == nullptr) {
        //     Mlib::verbose_abort("AEngine::InitDisplay: old window is null");
        // }
        UnloadResources();
        gl_context_->Invalidate();
        app_ = app;
        gl_context_->Init(app->window);
        UpdateDpi();
        LoadResources();
    } else {
        // initialize OpenGL ES and EGL
        gl_context_->Resume(app_->window);
        UnloadResources();
        LoadResources();
    }

    tap_camera_.SetFlip(1.f, -1.f, -1.f);
    tap_camera_.SetPinchTransformFactor(2.f, 2.f, 8.f);

    return 0;
}

void AEngine::DrawFrame(Mlib::RenderEvent event) {
    renderer_.render(
        event,
        LayoutParametersX(),
        LayoutParametersY());

    // Swap
    gl_context_->Swap();
}

/**
 * Tear down the EGL context currently associated with the display.
 */
void AEngine::SuspendContext() {
    gl_context_->Suspend();
}

bool AEngine::ContextIsSuspended() const {
    return gl_context_->IsSuspended();
}

void AEngine::InvalidateContext() {
    gl_context_->Invalidate();
}
/**
 * Process the next input event.
 */
int32_t AEngine::HandleInput(android_app* app, AInputEvent* event) {
    auto* eng = (AEngine*)app->userData;
    switch (AInputEvent_getType(event)) {
        case AINPUT_EVENT_TYPE_KEY: {
            eng->buttons_states_.notify_key_event(AKeyEvent_getKeyCode(event), AKeyEvent_getAction(event));
            break;
        }
        case AINPUT_EVENT_TYPE_MOTION: {
            std::scoped_lock lock{eng->buttons_states_.tap_buttons_.mutex};
            for (size_t i = 0; i < AMotionEvent_getPointerCount(event); ++i) {
                int32_t action_and_pointer_idX = AMotionEvent_getAction(event);
                int32_t pointer_idX = (action_and_pointer_idX & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
                int32_t pointer_id = AMotionEvent_getPointerId(event, i);
                int32_t action = action_and_pointer_idX & AMOTION_EVENT_ACTION_MASK;
                // Mlib::lraw() << "Combined " << action_and_pointer_idX << " " << action << " " << pointer_idX << " " << pointer_id;
                switch (action) {
                    case AMOTION_EVENT_ACTION_UP:
                    case AMOTION_EVENT_ACTION_POINTER_UP: {
                        // Mlib::lraw() << "Pointer ID " << pointer_id << " up, combined " << action_and_pointer_idX << " " << action << " " << pointer_idX << " " << pointer_id;
                        if ((action == AMOTION_EVENT_ACTION_POINTER_UP) && (pointer_id != pointer_idX)) {
                            // Mlib::lraw() << "skip";
                            break;
                        }
                        if (auto bit = eng->buttons_states_.tap_buttons_.button_pointer_ids.find(pointer_id);
                            (bit != eng->buttons_states_.tap_buttons_.button_pointer_ids.end()) &&
                            (bit->second != nullptr))
                        {
                            const auto &bs = *bit->second;
                            if (bs.key.has_value()) {
                                auto kit = eng->buttons_states_.tap_buttons_.button_down.find(
                                    *bs.key);
                                if (kit != eng->buttons_states_.tap_buttons_.button_down.end()) {
                                    kit->second = false;
                                }
                            }
                            bit->second = nullptr;
                        }
                        if (auto xit = eng->buttons_states_.tap_buttons_.joystick_xaxis_pointer_ids.find(pointer_id);
                            (xit != eng->buttons_states_.tap_buttons_.joystick_xaxis_pointer_ids.end()) &&
                            (xit->second != nullptr))
                        {
                            const auto &bs = *xit->second;
                            if (bs.joystick_xaxis.has_value()) {
                                auto jit = eng->buttons_states_.tap_buttons_.joystick_axis_position.find(
                                    *bs.joystick_xaxis);
                                if (jit !=
                                    eng->buttons_states_.tap_buttons_.joystick_axis_position.end()) {
                                    jit->second = NAN;
                                }
                            }
                            xit->second = nullptr;
                        }
                        if (auto yit = eng->buttons_states_.tap_buttons_.joystick_yaxis_pointer_ids.find(pointer_id);
                            (yit != eng->buttons_states_.tap_buttons_.joystick_yaxis_pointer_ids.end()) &&
                            (yit->second != nullptr))
                        {
                            const auto &bs = *yit->second;
                            if (bs.joystick_yaxis.has_value()) {
                                auto jit = eng->buttons_states_.tap_buttons_.joystick_axis_position.find(
                                    *bs.joystick_yaxis);
                                if (jit !=
                                    eng->buttons_states_.tap_buttons_.joystick_axis_position.end()) {
                                    jit->second = NAN;
                                }
                            }
                            yit->second = nullptr;
                        }
                        break;
                    }
                    case AMOTION_EVENT_ACTION_DOWN:
                    case AMOTION_EVENT_ACTION_MOVE:
                    case AMOTION_EVENT_ACTION_POINTER_DOWN: {
                        // if ((action == AMOTION_EVENT_ACTION_DOWN) ||
                        //     (action == AMOTION_EVENT_ACTION_POINTER_DOWN)) {
                        //     Mlib::lraw() << "Pointer ID " << pointer_id << " down, combined " << action_and_pointer_idX << " " << action << " " << pointer_idX << " " << pointer_id;
                        // }
                        if ((action == AMOTION_EVENT_ACTION_POINTER_DOWN) && (pointer_id != pointer_idX)) {
                            // Mlib::lraw() << "skip";
                            break;
                        }
                        float x = AMotionEvent_getX(event, i);
                        float y = AMotionEvent_getY(event, i);
                        for (auto &tb: eng->buttons_states_.tap_buttons_.button_states) {
                            auto ew = tb.widget->evaluate(
                                eng->LayoutParametersX(),
                                eng->LayoutParametersY(),
                                Mlib::YOrientation::SWAPPED,
                                Mlib::RegionRoundMode::ENABLED);
                            if ((ew->width() > 0.f) && (ew->height() > 0.f) &&
                                (x >= ew->left()) && (x <= ew->right()) &&
                                (y >= ew->bottom()) && (y <= ew->top()))
                            {
                                if (const auto &k = tb.key) {
                                    eng->buttons_states_.tap_buttons_.button_down[*k] = true;
                                    if ((action == AMOTION_EVENT_ACTION_DOWN) ||
                                        (action == AMOTION_EVENT_ACTION_POINTER_DOWN))
                                    {
                                        eng->buttons_states_.tap_buttons_.button_pointer_ids[pointer_id] = &tb;
                                    }
                                }
                                if (const auto &v = tb.joystick_xaxis) {
                                    eng->buttons_states_.tap_buttons_.joystick_axis_position[*v] =
                                        std::clamp(
                                            (2.f * x - (ew->left() + ew->right())) / ew->width(),
                                            -1.f, 1.f);
                                    if ((action == AMOTION_EVENT_ACTION_DOWN) ||
                                        (action == AMOTION_EVENT_ACTION_POINTER_DOWN))
                                    {
                                        eng->buttons_states_.tap_buttons_.joystick_xaxis_pointer_ids[pointer_id] = &tb;
                                    }
                                }
                                if (const auto &v = tb.joystick_yaxis) {
                                    eng->buttons_states_.tap_buttons_.joystick_axis_position[*v] =
                                        std::clamp(
                                            (2.f * y - (ew->bottom() + ew->top())) / ew->height(),
                                            -1.f, 1.f);
                                    if ((action == AMOTION_EVENT_ACTION_DOWN) ||
                                        (action == AMOTION_EVENT_ACTION_POINTER_DOWN))
                                    {
                                        eng->buttons_states_.tap_buttons_.joystick_yaxis_pointer_ids[pointer_id] = &tb;
                                    }
                                }
                            }
                        }
                        break;
                    }
                    default: ;// Do nothing
                }
            }
            break;
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
            // LOGI("APP_CMD_SAVE_STATE");
            break;
        case APP_CMD_INIT_WINDOW:
            // LOGI("APP_CMD_INIT_WINDOW");
            // The window is being shown, get it ready.
            if (app->window == nullptr) {
                Mlib::verbose_abort("APP_CMD_INIT_WINDOW: window is null");
            }
            eng->InitDisplay(app);
            eng->has_focus_ = true;
            eng->DrawFrame(Mlib::RenderEvent::INIT_WINDOW);
            break;
        case APP_CMD_TERM_WINDOW:
            // LOGI("APP_CMD_TERM_WINDOW");
        case APP_CMD_STOP:
            // LOGI("APP_CMD_STOP/APP_CMD_TERM_WINDOW");
            // The window is being hidden or closed, clean it up.
            eng->UnloadResources();
            eng->SuspendContext();
            eng->InvalidateContext();
            eng->has_focus_ = false;
            break;
        case APP_CMD_GAINED_FOCUS:
            // LOGI("APP_CMD_GAINED_FOCUS");
            eng->ResumeSensors();
            // Start animation
            if (eng->ContextIsSuspended()) {
                if (app->window == nullptr) {
                    Mlib::verbose_abort("APP_CMD_GAINED_FOCUS: window is null");
                }
                eng->InitDisplay(app);
            }
            eng->has_focus_ = true;
            eng->LoadResources();
            eng->DrawFrame(Mlib::RenderEvent::GAINED_FOCUS);
            break;
        case APP_CMD_LOST_FOCUS:
            // LOGI("APP_CMD_LOST_FOCUS");
            eng->UnloadResources();
            eng->SuspendSensors();
            // Also stop animating.
            eng->has_focus_ = false;
            break;
        case APP_CMD_LOW_MEMORY:
            // Free up GL resources
            // LOGI("APP_CMD_LOW_MEMORY");
            eng->UnloadResources();
            eng->InvalidateContext();
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
    return has_focus_ && (gl_context_->GetANativeWindow() != nullptr);
}

void AEngine::TransformPosition(ndk_helper::Vec2& vec) {
    vec = ndk_helper::Vec2(2.0f, 2.0f) * vec /
          ndk_helper::Vec2((float)gl_context_->GetScreenWidth(),
                           (float)gl_context_->GetScreenHeight()) -
          ndk_helper::Vec2(1.f, 1.f);
}

void AEngine::UpdateDpi() {
    JNIEnv* jni = ndk_helper::JNIHelper::GetInstance()->AttachCurrentThread();

    // Default class retrieval
    jclass clazz = jni->GetObjectClass(app_->activity->clazz);
    {
        jmethodID methodID = jni->GetMethodID(clazz, "xDpi", "()F");
        xdpi_ = jni->CallFloatMethod(app_->activity->clazz, methodID);
    }
    {
        jmethodID methodID = jni->GetMethodID(clazz, "yDpi", "()F");
        ydpi_ = jni->CallFloatMethod(app_->activity->clazz, methodID);
    }
}

Mlib::LayoutConstraintParameters AEngine::LayoutParametersX() const {
    return Mlib::LayoutConstraintParameters{
        .dpi = xdpi_,
        .min_pixel = 0.f,
        .end_pixel = (float)gl_context_->GetScreenWidth()};
}

Mlib::LayoutConstraintParameters AEngine::LayoutParametersY() const {
    return Mlib::LayoutConstraintParameters{
        .dpi = ydpi_,
        .min_pixel = 0.f,
        .end_pixel = (float)gl_context_->GetScreenHeight()};
}
