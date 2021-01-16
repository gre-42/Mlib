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

class RenderLogic {
public:
    virtual void render(
        int width,
        int height,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) = 0;
    virtual float near_plane() const {
        throw std::runtime_error("near_plane not implemented");
    }
    virtual float far_plane() const  {
        throw std::runtime_error("far_plane not implemented");
    }
    virtual const FixedArray<float, 4, 4>& vp() const  {
        throw std::runtime_error("vp not implemented");
    }
    virtual const TransformationMatrix<float, 3>& iv() const  {
        throw std::runtime_error("iv not implemented");
    }
    virtual bool requires_postprocessing() const  {
        throw std::runtime_error("requires_postprocessing not implemented");
    }
};

}
