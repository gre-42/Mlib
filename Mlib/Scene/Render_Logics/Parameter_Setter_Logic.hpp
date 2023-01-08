#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Regex.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Ui/List_View.hpp>
#include <Mlib/Scene_Graph/Focus_Filter.hpp>
#include <atomic>
#include <memory>
#include <vector>

namespace Mlib {

class ButtonPress;

struct ReplacementParameter {
    std::string name;
    SubstitutionMap substitutions;
};

class ParameterSetterLogic: public RenderLogic {
public:
    ParameterSetterLogic(
        const std::string& title,
        const std::vector<ReplacementParameter>& options,
        const std::string& ttf_filename,
        const FixedArray<float, 2>& position,
        const FixedArray<float, 2>& size,
        float font_height_pixels,
        float line_distance_pixels,
        const FocusFilter& focus_filter,
        SubstitutionMap& substitutions,
        ButtonPress& button_press,
        std::atomic_size_t& selection_index,
        const std::function<void()>& on_first_render = [](){},
        const std::function<void()>& on_change = [](){});
    ~ParameterSetterLogic();

    virtual void render(
        int width,
        int height,
        float xdpi,
        float ydpi,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    virtual FocusFilter focus_filter() const override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

private:
    void merge_substitutions() const;
    std::vector<ReplacementParameter> options_;
    ListView<ReplacementParameter> list_view_;
    FocusFilter focus_filter_;
    SubstitutionMap& substitutions_;
};

}
