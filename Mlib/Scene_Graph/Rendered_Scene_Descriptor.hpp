#pragma once
#include <Mlib/Geometry/Material/Render_Pass.hpp>
#include <Mlib/Scene_Graph/Render_Passes.hpp>
#include <Mlib/Scene_Graph/Render_Time_Id.hpp>

namespace Mlib {

struct RenderedSceneDescriptor {
    ExternalRenderPass external_render_pass = {.pass = ExternalRenderPassType::STANDARD};
    RenderTimeId time_id = 0;
    std::strong_ordering operator <=> (const RenderedSceneDescriptor&) const = default;
};

class RootRenderedSceneDescriptor {
public:
    inline const RenderedSceneDescriptor& next(
            bool motion_interpolation,
            std::chrono::steady_clock::time_point time)
    {
        rsd_.time_id = (rsd_.time_id + 1) % RENDER_TIME_ID_END;
        rsd_.external_render_pass.time = time;
        return rsd_;
    }
private:
    RenderedSceneDescriptor rsd_;
};

}
