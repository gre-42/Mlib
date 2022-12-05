#include "AUi.hpp"
#include <NDKHelper.h>

AUi::AUi(android_app& app)
: app_{app}
{}

void AUi::ShowMessage(
    const std::string& title,
    const std::string& message) const
{
    // LOGI("Title: %s, Message: %s", title.c_str(), message.c_str());
    JNIEnv* jni;
    app_.activity->vm->AttachCurrentThread(&jni, nullptr);

    // Default class retrieval
    jclass clazz = jni->GetObjectClass(app_.activity->clazz);
    jmethodID methodID = jni->GetMethodID(clazz, "showMessage", "(Ljava/lang/String;Ljava/lang/String;)V");
    jni->CallVoidMethod(
        app_.activity->clazz,
        methodID,
        jni->NewStringUTF(title.c_str()),
        jni->NewStringUTF(message.c_str()));

    app_.activity->vm->DetachCurrentThread();
}
