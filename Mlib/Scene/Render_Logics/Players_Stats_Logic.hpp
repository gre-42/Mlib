#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Layout/IWidget.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Text_Logic.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <memory>

namespace Mlib {

class Players;
class TextResource;
enum class ScoreBoardConfiguration;
class IWidget;

class PlayersStatsLogic: public RenderLogic, public RenderTextLogic {
public:
    PlayersStatsLogic(
        const Players& players,
        const std::string& ttf_filename,
        std::unique_ptr<IWidget>&& widget,
        const ILayoutScalar& font_height,
        const ILayoutScalar& line_distance,
        ScoreBoardConfiguration score_board_configuration);
    ~PlayersStatsLogic();

    virtual void render(
        int width,
        int height,
        float xdpi,
        float ydpi,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

private:
    const Players& players_;
    ScoreBoardConfiguration score_board_configuration_;
    std::unique_ptr<IWidget> widget_;
};

}
