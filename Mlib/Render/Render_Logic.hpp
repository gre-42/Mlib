#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <stdexcept>

namespace Mlib {

template <class TData, size_t tsize>
class TransformationMatrix;
struct RenderConfig;
struct RenderResults;
struct RenderedSceneDescriptor;
struct SceneGraphConfig;
enum class Focus;

class RenderLogic {
public:
    virtual void render(
        int width,
        int height,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) = 0;
    virtual Focus focus_mask() const;
    virtual float near_plane() const;
    virtual float far_plane() const;
    virtual const FixedArray<float, 4, 4>& vp() const;
    virtual const TransformationMatrix<float, 3>& iv() const;
    virtual bool requires_postprocessing() const;
};

}
