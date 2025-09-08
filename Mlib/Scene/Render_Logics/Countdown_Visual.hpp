#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Text_Logic.hpp>

namespace Mlib {

class CountdownPhysics;
class ExpressionWatcher;

class CountdownVisual final:
    public RenderLogic,
    public RenderTextLogic {
public:
    CountdownVisual(
        const DanglingBaseClassRef<CountdownPhysics>& physics,
        std::unique_ptr<ExpressionWatcher>&& ew,
        std::string charset,
        std::string ttf_filename,
        const FixedArray<float, 3>& color,
        const FixedArray<float, 2>& position,
        const ILayoutPixels& font_height,
        const ILayoutPixels& line_distance,
        std::string text);
    ~CountdownVisual();

    // RenderLogic
    virtual std::optional<RenderSetup> try_render_setup(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderedSceneDescriptor& frame_id) const override;
    virtual void render_without_setup(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

private:
    DestructionFunctionsRemovalTokens on_destroy_countdown_physics_;
    DanglingBaseClassRef<CountdownPhysics> physics_;
    std::unique_ptr<ExpressionWatcher> ew_;
    std::string charset_;
    std::string text_;
    FixedArray<float, 2> position_;
};

}
