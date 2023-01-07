#pragma once
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Ui/List_View.hpp>
#include <atomic>
#include <functional>
#include <vector>

namespace Mlib {

struct UiFocus;
struct SubmenuHeader;
class ButtonPress;
class ThreadSafeString;
class SubstitutionMap;

struct TabEntry {
    std::string title;
    std::unique_ptr<RenderLogic> content;
};

class TabMenuLogic: public RenderLogic {
public:
    TabMenuLogic(
        BaseKeyBinding key_binding,
        const std::string& title,
        const std::vector<SubmenuHeader>& options,
        const std::string& ttf_filename,
        const FixedArray<float, 2>& position,
        const FixedArray<float, 2>& size,
        float font_height_pixels,
        float line_distance_pixels,
        SubstitutionMap& substitutions,
        UiFocus& ui_focus,
        std::atomic_size_t& num_renderings,
        ButtonPress& button_press,
        std::atomic_size_t& selection_index,
        std::string previous_scene_filename,
        const ThreadSafeString& next_scene_filename,
        std::function<void()> reload_transient_objects,
        const std::function<void()>& on_change = [](){});
    ~TabMenuLogic();

    virtual void render(
        int width,
        int height,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    virtual FocusFilter focus_filter() const override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

private:
    BaseKeyBinding key_binding_;
    UiFocus& ui_focus_;
    ButtonPress& button_press_;
    std::string previous_scene_filename_;
    const ThreadSafeString& next_scene_filename_;
    std::atomic_size_t& num_renderings_;
    std::function<void()> reload_transient_objects_;
    ListView<SubmenuHeader> list_view_;
};

}
