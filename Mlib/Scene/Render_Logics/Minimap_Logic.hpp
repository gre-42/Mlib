#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Render/Data_Display/Centered_Texture_Image_Logic.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Render_Logics/Fill_With_Texture_Logic.hpp>
#include <memory>
#include <mutex>

namespace Mlib {

class SceneNode;
class IWidget;
class ILayoutPixels;

class MinimapLogic: public RenderLogic, public AdvanceTime {
public:
    MinimapLogic(
        SceneNode& node,
        const std::string& map_image_resource_name,
        const std::string& locator_image_resource_name,
        std::unique_ptr<IWidget>&& widget,
        const ILayoutPixels& locator_size,
        float pointer_reference_length,
        float scale,
        const FixedArray<float, 2>& size,
        const FixedArray<double, 2>& offset);
    ~MinimapLogic();

    // AdvanceTime
    virtual void advance_time(float dt) override;

    // RenderLogic
    virtual void render(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;

    virtual void print(std::ostream& ostr, size_t depth) const override;

private:
    SceneNode& node_;
    CenteredTextureImageLogic centered_texture_image_logic_;
    FillWithTextureLogic locator_logic_;
    std::unique_ptr<IWidget> widget_;
    const ILayoutPixels& locator_size_;
    float pointer_reference_length_;
    float scale_;
    FixedArray<float, 2> size_;
    FixedArray<double, 2> offset_;
    std::mutex pose_mutex_;
    FixedArray<double, 2> position_;
    float angle_;
};

}
