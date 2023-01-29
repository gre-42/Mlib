#pragma once

namespace Mlib {

struct LayoutConstraintParameters;

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
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly) = 0;
};

}
