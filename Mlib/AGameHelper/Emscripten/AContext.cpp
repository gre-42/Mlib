#include "AContext.hpp"
#include <emscripten/html5.h>

using namespace Mlib;

AContext::AContext() = default;

AContext::~AContext() = default;

bool AContext::is_initialized() const {
    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = emscripten_webgl_get_current_context();
    return context > 0;
}
