#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Physics/Interfaces/IAdvance_Time.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Text_Logic.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

namespace Mlib {

enum class Focus;
class Focuses;
class AdvanceTimes;
class ExpressionWatcher;

class CountDownLogic:
    public RenderLogic,
    public RenderTextLogic,
    public IAdvanceTime {
public:
    CountDownLogic(
        DanglingRef<SceneNode> node,
        std::unique_ptr<ExpressionWatcher>&& ew,
        std::string charset,
        std::string ttf_filename,
        const FixedArray<float, 3>& color,
        const FixedArray<float, 2>& position,
        const ILayoutPixels& font_height,
        const ILayoutPixels& line_distance,
        float duration,
        Focus pending_focus,
        Focus counting_focus,
        std::string text,
        Focuses& focuses);
    ~CountDownLogic();

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

    // IAdvanceTime
    virtual void advance_time(float dt, const StaticWorld& world) override;

    DestructionFunctionsRemovalTokens on_node_clear;
private:
    DanglingPtr<SceneNode> node_;
    std::unique_ptr<ExpressionWatcher> ew_;
    std::string charset_;
    float elapsed_time_;
    float duration_;
    Focus pending_focus_;
    Focus counting_focus_;
    std::string text_;
    Focuses& focuses_;
    FixedArray<float, 2> position_;
};

}
