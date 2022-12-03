#pragma once

namespace Mlib {

enum class RenderEvent {
    INIT_WINDOW,
    LOST_FOCUS,
    LOOP
};

class IRenderer {
public:
    virtual void init() = 0;
    virtual void unload() = 0;
    virtual void update_viewport() = 0;
    virtual void render(
        RenderEvent event,
        int width,
        int height) = 0;
};

}
