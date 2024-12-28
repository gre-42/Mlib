#include "AUi.hpp"
#include "AndroidApp.hpp"
#include <Mlib/Android/ndk_helper/JNIHelper.h>
#include <Mlib/Android/ndk_helper/NDKHelper.h>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Os/Set_Thread_Name_Native.hpp>
#include <fstream>
#include <sstream>

inline android_app& App() {
    return AndroidApp::App();
}

void AUi::ShowMessage(
    const std::string& title,
    const std::string& message)
{
    // LOGI("Title: %s, Message: %s", title.c_str(), message.c_str());
    JNIEnv* jni = ndk_helper::JNIHelper::GetInstance()->AttachCurrentThread();

    // Default class retrieval
    jclass clazz = jni->GetObjectClass(App().activity->clazz);
    jmethodID methodID = jni->GetMethodID(clazz, "showMessage", "(Ljava/lang/String;Ljava/lang/String;)V");
    jni->CallVoidMethod(
        App().activity->clazz,
        methodID,
        jni->NewStringUTF(title.c_str()),
        jni->NewStringUTF(message.c_str()));
}

void AUi::RequestReadExternalStoragePermission() {
    JNIEnv* jni = ndk_helper::JNIHelper::GetInstance()->AttachCurrentThread();

    // Default class retrieval
    jclass clazz = jni->GetObjectClass(App().activity->clazz);
    jmethodID methodID = jni->GetMethodID(clazz, "requestReadExternalStoragePermission", "()V");
    jni->CallVoidMethod(
        App().activity->clazz,
        methodID);
}

std::unique_ptr<std::istream> AUi::OpenFile(
    const std::filesystem::path& filename,
    std::ios_base::openmode mode)
{
    std::vector<uint8_t> buffer;
    if (!ndk_helper::JNIHelper::GetInstance()->ReadFile(filename.lexically_normal().c_str(), &buffer)) {
        auto res = std::make_unique<std::istringstream>(mode);
        res->setstate(std::ios::failbit);
        return res;
    }
    return std::make_unique<std::istringstream>(
        std::string((char*)buffer.data(), buffer.size()));
}

std::vector<uint8_t> AUi::ReadFile(const std::filesystem::path& filename) {
    std::vector<uint8_t> buffer;
    if (!ndk_helper::JNIHelper::GetInstance()->ReadFile(filename.lexically_normal().c_str(), &buffer)) {
        Mlib::verbose_abort("Could not read from file \"" + filename.string() + '"');
    }
    return buffer;
}

bool AUi::PathExists(const std::filesystem::path& path) {
    return ndk_helper::JNIHelper::GetInstance()->PathExists(path.lexically_normal().c_str());
}

ndk_helper::DirectoryIterator AUi::ListDir(const std::filesystem::path& dirname) {
    return ndk_helper::JNIHelper::GetInstance()->ListDir(dirname.lexically_normal().c_str());
}

std::string AUi::GetFilesDir(ndk_helper::StorageType storage_type) {
    return ndk_helper::JNIHelper::GetInstance()->GetFilesDir(storage_type);
}

// From: https://stackoverflow.com/questions/12702868/how-to-force-landscape-mode-with-ndk-using-pure-c-codes
void AUi::SetRequestedScreenOrientation(ScreenOrientation orientation) {
    JNIEnv* jni = ndk_helper::JNIHelper::GetInstance()->AttachCurrentThread();
    jclass clazz = jni->GetObjectClass(App().activity->clazz);
    jmethodID methodID = jni->GetMethodID(clazz, "setRequestedOrientation", "(I)V");
    jni->CallVoidMethod(App().activity->clazz, methodID, (int)orientation);
}

std::string AUi::GetFlavor() {
    JNIEnv* jni = ndk_helper::JNIHelper::GetInstance()->AttachCurrentThread();
    jclass clazz = jni->GetObjectClass(App().activity->clazz);
    jmethodID methodID = jni->GetMethodID(clazz, "flavor", "()Ljava/lang/String;");
    auto flavorJstring = (jstring)jni->CallObjectMethod(App().activity->clazz, methodID);
    const char* flavorUtf = jni->GetStringUTFChars(flavorJstring, nullptr);
    auto result = std::string{flavorUtf};
    jni->ReleaseStringUTFChars(flavorJstring, flavorUtf);
    jni->DeleteLocalRef(flavorJstring);
    return result;
}

void AUi::SetThreadName(const std::string& name) {
    // From: https://stackoverflow.com/questions/50327631/android-ndk-setting-a-name-for-a-c-thread
    ndk_helper::JNIHelper::GetInstance()->AttachCurrentThread();
    Mlib::set_thread_name_native(name);
}
