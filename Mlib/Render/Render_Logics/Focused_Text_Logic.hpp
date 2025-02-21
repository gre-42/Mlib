#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Text_Logic.hpp>

namespace Mlib {

template <class T>
class VariableAndHash;
enum class Focus;

class FocusedTextLogic: public RenderLogic, public RenderTextLogic {
public:
    FocusedTextLogic(
        VariableAndHash<std::string> charset,
        std::string ttf_filename,
        const FixedArray<float, 3>& color,
        const FixedArray<float, 2>& position,
        const ILayoutPixels& font_height,
        const ILayoutPixels& line_distance,
        Focus focus_mask,
        std::string text);
    ~FocusedTextLogic();

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
    const std::string text_;
    Focus focus_mask_;
};

}
