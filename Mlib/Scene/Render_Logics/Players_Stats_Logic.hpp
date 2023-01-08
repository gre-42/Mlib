#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Text_Logic.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>

namespace Mlib {

class Players;
class TextResource;
enum class ScoreBoardConfiguration;
enum class ScreenUnits;

class PlayersStatsLogic: public RenderLogic, public RenderTextLogic {
public:
    PlayersStatsLogic(
        const Players& players,
        const std::string& ttf_filename,
        const FixedArray<float, 2>& position,
        const FixedArray<float, 2>& size,
        float font_height,
        float line_distance,
        ScreenUnits units,
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
    FixedArray<float, 2> size_;
};

}
