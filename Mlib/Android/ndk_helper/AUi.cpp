#include "AUi.hpp"
#include <NDKHelper.h>
#include <fstream>
#include <sstream>

[[ noreturn ]] static void verbose_abort(const std::string& message) {
    LOGE("Aborting: %s", message.c_str());
    std::abort();
}

#define HELPER_CLASS_NAME \
  "com/hallo2hallo/helper/NDKHelper"  // Class name of helper function

android_app* AUi::app_ = nullptr;

void AUi::Init(android_app& app)
{
    if (app_ != nullptr) {
        verbose_abort("AUi already initialized");
    }
    app_ = &app;
    ndk_helper::JNIHelper::Init(app.activity, HELPER_CLASS_NAME);
}

void AUi::Destroy() {
    if (app_ == nullptr) {
        verbose_abort("AUi already destroyed");
    }
    app_ = nullptr;
}

android_app& AUi::App() {
    if (app_ == nullptr) {
        verbose_abort("AUi not initialized");
    }
    return *app_;
}

void AUi::ShowMessage(
    const std::string& title,
    const std::string& message)
{
    // LOGI("Title: %s, Message: %s", title.c_str(), message.c_str());
    JNIEnv* jni;
    App().activity->vm->AttachCurrentThread(&jni, nullptr);

    // Default class retrieval
    jclass clazz = jni->GetObjectClass(App().activity->clazz);
    jmethodID methodID = jni->GetMethodID(clazz, "showMessage", "(Ljava/lang/String;Ljava/lang/String;)V");
    jni->CallVoidMethod(
        App().activity->clazz,
        methodID,
        jni->NewStringUTF(title.c_str()),
        jni->NewStringUTF(message.c_str()));

    App().activity->vm->DetachCurrentThread();
}

void AUi::RequestReadExternalStoragePermission() {
    JNIEnv* jni;
    App().activity->vm->AttachCurrentThread(&jni, nullptr);

    // Default class retrieval
    jclass clazz = jni->GetObjectClass(App().activity->clazz);
    jmethodID methodID = jni->GetMethodID(clazz, "requestReadExternalStoragePermission", "()V");
    jni->CallVoidMethod(
        App().activity->clazz,
        methodID);

    App().activity->vm->DetachCurrentThread();
}

std::unique_ptr<std::istream> AUi::OpenFile(
    const std::filesystem::path& filename,
    std::ios_base::openmode mode)
{
    std::vector<uint8_t> buffer;
    if (!ndk_helper::JNIHelper::GetInstance()->ReadFile(filename.c_str(), &buffer)) {
        auto res = std::make_unique<std::istringstream>(mode);
        res->setstate(std::ios::failbit);
        return res;
    }
    return std::make_unique<std::istringstream>(
        std::string((char*)buffer.data(), buffer.size()));
}

std::vector<uint8_t> AUi::ReadFile(const std::filesystem::path& filename) {
    std::vector<uint8_t> buffer;
    if (!ndk_helper::JNIHelper::GetInstance()->ReadFile(filename.c_str(), &buffer)) {
        verbose_abort("Could not read from file \"" + filename.string() + '"');
    }
    return buffer;
}

bool AUi::PathExists(const std::filesystem::path& path) {
    return ndk_helper::JNIHelper::GetInstance()->PathExists(path.c_str());
}

ndk_helper::DirectoryIterator AUi::ListDir(const std::filesystem::path& dirname) {
    return ndk_helper::JNIHelper::GetInstance()->ListDir(dirname.c_str());
}

std::string AUi::GetExternalFilesDir() {
    return ndk_helper::JNIHelper::GetInstance()->GetExternalFilesDir();
}

// From: https://stackoverflow.com/questions/12702868/how-to-force-landscape-mode-with-ndk-using-pure-c-codes
void AUi::SetRequestedScreenOrientation(ScreenOrientation orientation) {
    JNIEnv* jni;
    App().activity->vm->AttachCurrentThread(&jni, nullptr);
    jclass clazz = jni->GetObjectClass(App().activity->clazz);
    jmethodID methodID = jni->GetMethodID(clazz, "setRequestedOrientation", "(I)V");
    jni->CallVoidMethod(App().activity->clazz, methodID, (int)orientation);
    App().activity->vm->DetachCurrentThread();
}

AUiGuard::AUiGuard(android_app& app) {
    AUi::Init(app);
}

AUiGuard::~AUiGuard() {
    AUi::Destroy();
}
