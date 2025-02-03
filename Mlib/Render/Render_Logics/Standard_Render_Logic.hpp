#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Threads/Fast_Mutex.hpp>
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

    virtual std::optional<RenderSetup> try_render_setup(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderedSceneDescriptor& frame_id) const override;
    virtual void render_with_setup(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id,
        const RenderSetup& setup) override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

    void set_background_color(const FixedArray<float, 3>& color);
private:
    const Scene& scene_;
    RenderLogic& child_logic_;
    FixedArray<float, 3> background_color_;
    ClearMode clear_mode_;
    mutable FastMutex mutex_;
};

}
