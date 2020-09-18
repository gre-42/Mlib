#pragma once
#include <Mlib/Array/Array_Forward.hpp>

namespace Mlib {

struct RenderConfig;
struct RenderResults;
struct RenderedSceneDescriptor;
struct SceneGraphConfig;

class RenderLogic {
public:
    virtual void render(
        int width,
        int height,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) = 0;
    virtual float near_plane() const = 0;
    virtual float far_plane() const = 0;
    virtual const FixedArray<float, 4, 4>& vp() const = 0;
    virtual const FixedArray<float, 4, 4>& iv() const = 0;
    virtual bool requires_postprocessing() const = 0;
};

}
