#include "AContext.hpp"
#include <Mlib/Android/ndk_helper/GLContext.h>
#include <Mlib/Throw_Or_Abort.hpp>

AContext::AContext()
: main_thread_looper_{ALooper_forThread()}
{
    if (main_thread_looper_ == nullptr) {
        THROW_OR_ABORT("Main thread looper is null");
    }
}

AContext::~AContext() = default;

bool AContext::is_initialized() const {
    return
        (ALooper_forThread() == main_thread_looper_) &&
        ndk_helper::GLContext::GetInstance()->IsInitialized();
}
