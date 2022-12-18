#include "Context_Obtainer.hpp"
#include <Mlib/Render/IWindow.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <stdexcept>

using namespace Mlib;

IWindow* ContextObtainer::window_ = nullptr;

bool ContextObtainer::is_initialized() {
    return window().is_initialized();
}

IWindow& ContextObtainer::window() {
    if (window_ == nullptr) {
        THROW_OR_ABORT("Global window not set");
    }
    return *window_;
}

void ContextObtainer::set_window(IWindow& window) {
    if (window_ != nullptr) {
        THROW_OR_ABORT("Global window already set");
    }
    window_ = &window;

}
