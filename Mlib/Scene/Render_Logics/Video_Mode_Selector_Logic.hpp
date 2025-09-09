#pragma once
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Macro_Executor/Focus_Filter.hpp>
#include <Mlib/Regex/Misc.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Ui/IList_View_Contents.hpp>
#include <Mlib/Render/Ui/List_View.hpp>
#include <cstdint>
#include <memory>
#include <vector>

namespace Mlib {

class UiFocus;
class ButtonStates;
class IWidget;
class ILayoutPixels;
template <typename TData, size_t... tshape>
class FixedArray;
class ExpressionWatcher;
class JsonView;
struct VideoMode;
class WindowLogic;
class ButtonPress;

class VideoModeContents: public IListViewContents {
public:
    explicit VideoModeContents(
        const std::vector<VideoMode>& video_modes);

    // IListViewContents
    virtual size_t num_entries() const override;
    virtual bool is_visible(size_t index) const override;
private:
    const std::vector<VideoMode>& video_modes_;
};

class VideoModeSelectorLogic: public RenderLogic {
public:
    VideoModeSelectorLogic(
        std::string id,
        WindowLogic& window_logic,
        ButtonPress& confirm_button,
        std::string charset,
        std::string ttf_filename,
        std::unique_ptr<IWidget>&& widget,
        const FixedArray<float, 3>& font_color,
        const ILayoutPixels& font_height,
        const ILayoutPixels& line_distance,
        FocusFilter focus_filter,
        std::unique_ptr<ExpressionWatcher>&& ew,
        ButtonStates& button_states,
        UiFocus& ui_focus,
        uint32_t user_id);
    ~VideoModeSelectorLogic();

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
    virtual bool is_visible(const UiFocus& ui_focus) const override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

private:
    std::unique_ptr<ExpressionWatcher> ew_;
    ButtonPress& confirm_button_;
    std::string charset_;
    std::string globals_prefix_;
    std::unique_ptr<TextResource> renderable_text_;
    std::vector<VideoMode> video_modes_;
    VideoModeContents contents_;
    std::unique_ptr<IWidget> widget_;
    const ILayoutPixels& font_height_;
    const ILayoutPixels& line_distance_;
    FocusFilter focus_filter_;
    UiFocus& ui_focus_;
    std::string id_;
    WindowLogic& window_logic_;
    ListView list_view_;
};

}
