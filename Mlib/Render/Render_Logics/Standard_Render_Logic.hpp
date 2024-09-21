#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Threads/Atomic_Mutex.hpp>
#include <mutex>

namespace Mlib {

class Scene;
enum class ClearMode;

class StandardRenderLogic: public RenderLogic {
public:
    StandardRenderLogic(
        const Scene& scene,
        RenderLogic& child_logic,
        const FixedArray<float, 3>& background_color,
        ClearMode clear_mode);
    ~StandardRenderLogic();

    virtual void init(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderedSceneDescriptor& frame_id) override;
    virtual void render(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    virtual void reset() override;
    virtual float near_plane() const override;
    virtual float far_plane() const override;
    virtual const FixedArray<ScenePos, 4, 4>& vp() const override;
    virtual const TransformationMatrix<float, ScenePos, 3>& iv() const override;
    virtual DanglingPtr<const SceneNode> camera_node() const override;
    virtual bool requires_postprocessing() const override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

    void set_background_color(const FixedArray<float, 3>& color);
private:
    const Scene& scene_;
    RenderLogic& child_logic_;
    FixedArray<float, 3> background_color_;
    ClearMode clear_mode_;
    mutable AtomicMutex mutex_;
};

}
