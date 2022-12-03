#include "AWindow.hpp"
#include <Mlib/Android/ndk_helper/GLContext.h>

AWindow::AWindow(ANativeWindow& native_window)
: native_window_{native_window}
{}

AWindow::~AWindow() {
    ndk_helper::GLContext::GetInstance()->Invalidate();
}

void AWindow::make_current() const {
    ndk_helper::GLContext::GetInstance()->Resume(&native_window_);
}

void AWindow::unmake_current() const {
    ndk_helper::GLContext::GetInstance()->Suspend();
}

bool AWindow::is_initialized() const {
    return ndk_helper::GLContext::GetInstance()->IsInitialized();
}
