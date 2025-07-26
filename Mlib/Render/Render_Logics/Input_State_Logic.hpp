#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Macro_Executor/Focus_Filter.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Text_Logic.hpp>
#include <chrono>
#include <memory>

namespace Mlib {

class ExpressionWatcher;
class ButtonStates;
class IWidget;

class InputStateLogic: public RenderLogic, public RenderTextLogic {
public:
    InputStateLogic(
        std::unique_ptr<ExpressionWatcher>&& ew,
        std::string charset,
        std::string ttf_filename,
        std::unique_ptr<IWidget>&& widget,
        const FixedArray<float, 3>& font_color,
        const ILayoutPixels& font_height,
        const ILayoutPixels& line_distance,
        FocusFilter focus_filter,
        std::chrono::milliseconds update_interval,
        const ButtonStates& button_states);
    ~InputStateLogic();

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
    virtual FocusFilter focus_filter() const override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

private:
    std::unique_ptr<ExpressionWatcher> ew_;
    std::string charset_;
    std::unique_ptr<IWidget> widget_;
    std::chrono::steady_clock::time_point last_update_;
    std::chrono::milliseconds update_interval_;
    std::string text_;
    const ButtonStates& button_states_;
    FocusFilter focus_filter_;
};

}
