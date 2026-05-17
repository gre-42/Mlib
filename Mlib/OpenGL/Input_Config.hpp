#pragma once

namespace Mlib {

struct InputConfig {
#if !defined(__EMSCRIPTEN__) && !defined(__ANDROID__)
    float polling_interval_seconds = 1.f / 240.f;
#endif
    bool show_mouse_cursor = true;
};

}
