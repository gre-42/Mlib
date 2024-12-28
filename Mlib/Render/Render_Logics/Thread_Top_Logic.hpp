#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Text_Logic.hpp>
#include <chrono>

namespace Mlib {

enum class Focus;

class ThreadTopLogic: public RenderLogic, public RenderTextLogic {
public:
    ThreadTopLogic(
        const std::string& ttf_filename,
        const FixedArray<float, 3>& color,
        const FixedArray<float, 2>& position,
        const ILayoutPixels& font_height,
        const ILayoutPixels& line_distance,
        std::chrono::milliseconds update_interval,
        Focus focus_mask);
    ~ThreadTopLogic();

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
    const FixedArray<float, 2> position_;
    std::chrono::steady_clock::time_point last_update_;
    std::chrono::milliseconds update_interval_;
    std::string text_;
    Focus focus_mask_;
};

}
