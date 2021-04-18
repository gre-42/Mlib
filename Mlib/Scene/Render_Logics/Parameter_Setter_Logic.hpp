#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Regex.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Ui/ListView.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <memory>
#include <vector>

namespace Mlib {

class ButtonPress;

struct ReplacementParameter {
    std::string name;
    SubstitutionString substitutions;
};

class ParameterSetterLogic: public RenderLogic {
public:
    ParameterSetterLogic(
        const std::vector<ReplacementParameter>& options,
        const std::string& ttf_filename,
        const FixedArray<float, 2>& position,
        float font_height_pixels,
        float line_distance_pixels,
        UiFocus& ui_focus,
        size_t submenu_id,
        SubstitutionString& substitutions,
        size_t& num_renderings,
        ButtonPress& button_press,
        size_t& selection_index,
        const std::string& previous_scene_filename,
        const std::string& next_scene_filename,
        const std::function<void()>& reload_transient_objects,
        const std::function<void()>& on_change = [](){});
    ~ParameterSetterLogic();

    virtual void render(
        int width,
        int height,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;

    virtual Focus focus_mask() const override;
private:
    void merge_substitutions() const;
    ListView<ReplacementParameter> scene_selector_list_view_;
    UiFocus& ui_focus_;
    size_t submenu_id_;
    SubstitutionString& substitutions_;
    size_t& num_renderings_;
    ButtonPress& button_press_;
    const std::string& previous_scene_filename_;
    const std::string& next_scene_filename_;
    std::function<void()> reload_transient_objects_;
};

}
