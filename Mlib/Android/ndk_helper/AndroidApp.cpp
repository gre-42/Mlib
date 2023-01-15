#include "AndroidApp.hpp"
#include <NDKHelper.h>
#include <string>

#define HELPER_CLASS_NAME \
  "com/hallo2hallo/helper/NDKHelper"  // Class name of helper function

[[ noreturn ]] static void verbose_abort(const std::string& message) {
    LOGE("Aborting: %s", message.c_str());
    std::abort();
}

android_app* AndroidApp::app_ = nullptr;

void AndroidApp::Init(android_app& app)
{
    if (app_ != nullptr) {
        verbose_abort("AndroidApp already initialized");
    }
    app_ = &app;
    ndk_helper::JNIHelper::Init(app.activity, HELPER_CLASS_NAME);
}

void AndroidApp::Destroy() {
    if (app_ == nullptr) {
        verbose_abort("AndroidApp already destroyed");
    }
    app_ = nullptr;
}

android_app& AndroidApp::App() {
    if (app_ == nullptr) {
        verbose_abort("AndroidApp not initialized");
    }
    return *app_;
}

AndroidAppGuard::AndroidAppGuard(android_app& app) {
    AndroidApp::Init(app);
}

AndroidAppGuard::~AndroidAppGuard() {
    AndroidApp::Destroy();
}
