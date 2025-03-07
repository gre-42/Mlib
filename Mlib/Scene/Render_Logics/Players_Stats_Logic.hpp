#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Layout/IWidget.hpp>
#include <Mlib/Macro_Executor/Focus_Filter.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Text_Logic.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <cstddef>
#include <memory>

namespace Mlib {

class Players;
class TextResource;
enum class ScoreBoardConfiguration;
class IWidget;
template <typename TData, size_t... tshape>
class FixedArray;
class ExpressionWatcher;

class PlayersStatsLogic: public RenderLogic, public RenderTextLogic {
public:
    PlayersStatsLogic(
        const Players& players,
        std::unique_ptr<ExpressionWatcher>&& ew,
        std::string charset,
        std::string ttf_filename,
        std::unique_ptr<IWidget>&& widget,
        const FixedArray<float, 3>& font_color,
        const ILayoutPixels& font_height,
        const ILayoutPixels& line_distance,
        ScoreBoardConfiguration score_board_configuration,
        FocusFilter focus_filter);
    ~PlayersStatsLogic();

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
    virtual FocusFilter focus_filter() const override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

private:
    const Players& players_;
    std::unique_ptr<ExpressionWatcher> ew_;
    std::string charset_;
    ScoreBoardConfiguration score_board_configuration_;
    std::unique_ptr<IWidget> widget_;
    FocusFilter focus_filter_;
};

}
