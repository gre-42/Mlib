#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <iosfwd>

namespace Mlib {

template <class TDir, class TPos, size_t n>
class TransformationMatrix;
struct RenderConfig;
struct RenderResults;
struct RenderedSceneDescriptor;
struct SceneGraphConfig;
struct FocusFilter;
class SceneNode;

class RenderLogic {
public:
    virtual ~RenderLogic();
    virtual void render(
        int width,
        int height,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) = 0;
    virtual FocusFilter focus_filter() const;
    virtual float near_plane() const;
    virtual float far_plane() const;
    virtual const FixedArray<double, 4, 4>& vp() const;
    virtual const TransformationMatrix<float, double, 3>& iv() const;
    virtual const SceneNode& camera_node() const;
    virtual bool requires_postprocessing() const;
    virtual void print(std::ostream& ostr, size_t depth) const = 0;
};

}
