#include "AContext.hpp"
#include <Mlib/Android/ndk_helper/GLContext.h>

AContext::AContext() = default;

AContext::~AContext() = default;

bool AContext::is_initialized() const {
    return ndk_helper::GLContext::GetInstance()->IsInitialized();
}
