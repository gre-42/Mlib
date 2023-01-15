#pragma once

namespace Mlib {

enum class RenderEvent {
    INIT_WINDOW,
    GAINED_FOCUS,
    LOOP
};

class IRenderer {
public:
    virtual void load_resources() = 0;
    virtual void unload_resources() = 0;
    virtual void render(
        RenderEvent event,
        int width,
        int height,
        float xdpi,
        float ydpi) = 0;
};

}
