#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Text_Logic.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>

namespace Mlib {

class Players;
class RenderableText;

class PlayersStatsLogic: public RenderLogic, public RenderTextLogic {
public:
    PlayersStatsLogic(
        const Players& players,
        const std::string& ttf_filename,
        const FixedArray<float, 2>& position,
        float font_height_pixels,
        float line_distance_pixels);
    ~PlayersStatsLogic();

    virtual void render(
        int width,
        int height,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    virtual float near_plane() const override;
    virtual float far_plane() const override;
    virtual const FixedArray<float, 4, 4>& vp() const override;
    virtual bool requires_postprocessing() const override;

private:
    const Players& players_;
};

}
