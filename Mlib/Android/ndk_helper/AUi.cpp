#include "AUi.hpp"
#include <NDKHelper.h>
#include <fstream>
#include <sstream>

#define HELPER_CLASS_NAME \
  "com/hallo2hallo/helper/NDKHelper"  // Class name of helper function

android_app* AUi::app_ = nullptr;

void AUi::Init(android_app& app)
{
    if (app_ != nullptr) {
        throw std::runtime_error("AUi already initialized");
    }
    app_ = &app;
    ndk_helper::JNIHelper::Init(app.activity, HELPER_CLASS_NAME);
}

android_app& AUi::App() {
    if (app_ == nullptr) {
        throw std::runtime_error("AUi not initialized");
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

std::unique_ptr<std::istream> AUi::OpenFile(const std::string& filename)
{
    std::vector<uint8_t> buffer;
    if (!ndk_helper::JNIHelper::GetInstance()->ReadFile(filename.c_str(), &buffer)) {
        auto res = std::make_unique<std::istringstream>();
        res->setstate(std::ios::failbit);
        return res;
    }
    return std::make_unique<std::istringstream>(
        std::string((char*)buffer.data(), buffer.size()));
}

bool AUi::PathExists(const std::string& path) {
    return ndk_helper::JNIHelper::GetInstance()->PathExists(path.c_str());
}

ndk_helper::DirectoryIterator AUi::ListDir(const std::string& dirname) {
    return ndk_helper::JNIHelper::GetInstance()->ListDir(dirname.c_str());
}
