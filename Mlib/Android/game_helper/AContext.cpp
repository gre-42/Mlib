#include "AContext.hpp"
#include <Mlib/Android/ndk_helper/GLContext.h>
#include <Mlib/Throw_Or_Abort.hpp>

AContext::AContext()
: main_thread_id_{std::this_thread::get_id()}
{}

AContext::~AContext() = default;

bool AContext::is_initialized() const {
    return
        (std::this_thread::get_id() == main_thread_id_) &&
        ndk_helper::GLContext::GetInstance()->IsInitialized();
}
