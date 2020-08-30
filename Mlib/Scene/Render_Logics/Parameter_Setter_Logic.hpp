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
        bool& leave_render_loop,
        ButtonPress& button_press);
    ~ParameterSetterLogic();

    virtual void initialize(GLFWwindow* window) override;
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
    ListView<ReplacementParameter> scene_selector_list_view_;
    UiFocus& ui_focus_;
    size_t submenu_id_;
    SubstitutionString& substitutions_;
    bool& leave_render_loop_;
    ButtonPress& button_press_;
    GLFWwindow* window_;
};

}
