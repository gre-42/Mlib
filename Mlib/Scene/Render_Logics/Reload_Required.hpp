#pragma once
#include <Mlib/Macro_Executor/Focus_Filter.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Text_Logic.hpp>
#include <memory>

namespace Mlib {

template <class T>
class VariableAndHash;
class UiFocus;
class IWidget;
template <typename TData, size_t... tshape>
class FixedArray;
class ExpressionWatcher;

class ReloadRequired:
    public RenderLogic,
    public RenderTextLogic
{
public:
    ReloadRequired(
        std::string title,
        std::string charset,
        std::string ttf_filename,
        std::unique_ptr<IWidget>&& widget,
        const FixedArray<float, 3>& font_color,
        const ILayoutPixels& font_height,
        const ILayoutPixels& line_distance,
        FocusFilter focus_filter,
        std::unique_ptr<ExpressionWatcher>&& ew,
        UiFocus& ui_focus);
    virtual ~ReloadRequired();

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
    virtual bool is_visible(const UiFocus& ui_focus) const override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

private:
    std::string title_;
    std::string charset_;
    std::string cached_title_;
    std::unique_ptr<ExpressionWatcher> ew_;
    UiFocus& ui_focus_;
    std::unique_ptr<IWidget> widget_;
    FocusFilter focus_filter_;
};

}
