#include "Minimap_Logic.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Geometry/Material/Color_Mode.hpp>
#include <Mlib/Geometry/Material/Cull_Face_Mode.hpp>
#include <Mlib/Layout/ILayout_Pixels.hpp>
#include <Mlib/Layout/Layout_Constraint_Parameters.hpp>
#include <Mlib/Layout/Screen_Units.hpp>
#include <Mlib/Layout/Widget.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Render_Logics/Clear_Mode.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Render_Logics/Resource_Update_Cycle.hpp>
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <sstream>

using namespace Mlib;

MinimapLogic::MinimapLogic(
    ObjectPool& object_pool,
    AdvanceTimes& advance_times,
    RenderLogics& render_logics,
    const DanglingBaseClassRef<Player>& player,
    const DanglingRef<SceneNode>& node,
    const VariableAndHash<std::string>& map_image_resource_name,
    const VariableAndHash<std::string>& locator_image_resource_name,
    std::unique_ptr<IWidget>&& widget,
    const ILayoutPixels& locator_size,
    float pointer_reference_length,
    float scale,
    const FixedArray<float, 2>& size,
    const FixedArray<double, 2>& offset)
    : render_logics_{ render_logics }
    , node_{ node }
    , centered_texture_image_logic_{
          RenderingContextStack::primary_rendering_resources().get_texture_lazy(
              ColormapWithModifiers{
                  .filename = map_image_resource_name,
                  .color_mode = ColorMode::RGB,
                  .mipmap_mode = MipmapMode::WITH_MIPMAPS
              }.compute_hash()),
          ContinuousBlendMode::ADD
    }
    , locator_logic_{
          RenderingContextStack::primary_rendering_resources().get_texture_lazy(
              ColormapWithModifiers{
                  .filename = locator_image_resource_name,
                  .color_mode = ColorMode::RGBA,
                  .mipmap_mode = MipmapMode::WITH_MIPMAPS
              }.compute_hash()),
    }
    , widget_{ std::move(widget) }
    , locator_size_{ locator_size }
    , pointer_reference_length_{ pointer_reference_length }
    , scale_{ scale }
    , size_{ size }
    , offset_{ offset }
    , position_{ fixed_nans<double, 2>() }
    , angle_{ NAN }
    , on_player_delete_vehicle_internals_{ player->delete_vehicle_internals, CURRENT_SOURCE_LOCATION }
{
    advance_times.add_advance_time({ *this, CURRENT_SOURCE_LOCATION }, CURRENT_SOURCE_LOCATION);
    on_player_delete_vehicle_internals_.add([this, &object_pool]() { object_pool.remove(*this); }, CURRENT_SOURCE_LOCATION);
    render_logics_.append({ *this, CURRENT_SOURCE_LOCATION }, 0 /* z_order */, CURRENT_SOURCE_LOCATION);
}

MinimapLogic::~MinimapLogic() {
    on_destroy.clear();
}

void MinimapLogic::advance_time(float dt, const StaticWorld& world) {
    std::scoped_lock lock{pose_mutex_};
    auto t = node_->absolute_model_matrix();
    position_ = {t.t(0), t.t(2)};
    angle_ = std::atan2(t.R(2, 0), t.R(2, 2));
}

std::optional<RenderSetup> MinimapLogic::try_render_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderedSceneDescriptor& frame_id) const
{
    return std::nullopt;
}

void MinimapLogic::render_without_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("MinimapLogic::render");
    FixedArray<double, 2> pos = uninitialized;
    float angle;
    {
        std::scoped_lock lock{pose_mutex_};
        pos = position_;
        angle = angle_;
    }
    if (std::isnan(angle)) {
        return;
    }
    auto pixel_region = widget_->evaluate(lx, ly, YOrientation::AS_IS, RegionRoundMode::ENABLED);
    {
        auto vg = ViewportGuard::from_widget(*pixel_region);
        if (vg.has_value()) {
            float aspect_ratio = pixel_region->width() / pixel_region->height();
            auto canvas_size = FixedArray<float, 2>{aspect_ratio, 1.f} * pointer_reference_length_;
            FixedArray<double, 2> p00_r{0.f, 0.f};
            FixedArray<double, 2> p10_r{size_(0), 0.f};
            FixedArray<double, 2> p01_r{0.f, size_(1)};
            FixedArray<double, 2> p11_r{size_(0), size_(1)};
            auto pc = (offset_ + pos) / (double)scale_;
            auto p00 = (p00_r - pc).casted<float>();
            auto p10 = (p10_r - pc).casted<float>();
            auto p01 = (p01_r - pc).casted<float>();
            auto p11 = (p11_r - pc).casted<float>();
            centered_texture_image_logic_.render(
                canvas_size,
                -angle,
                FixedArray<float, 2, 2, 2>::init(
                    p00(0), p01(0),
                    p10(0), p11(0),
                    p00(1), p01(1),
                    p10(1), p11(1)));
        }
    }
    {
        auto center = FixedArray<float, 2>{
            (pixel_region->left() + pixel_region->right()) / 2.f,
            (pixel_region->bottom() + pixel_region->top()) / 2.f};
        PixelRegion locator_pixel_region{
            center(0) - locator_size_.to_pixels(lx, PixelsRoundMode::NONE) / 2.f,
            center(0) + locator_size_.to_pixels(lx, PixelsRoundMode::NONE) / 2.f,
            center(1) - locator_size_.to_pixels(ly, PixelsRoundMode::NONE) / 2.f,
            center(1) + locator_size_.to_pixels(ly, PixelsRoundMode::NONE) / 2.f,
            RegionRoundMode::ENABLED};
        auto vg = ViewportGuard::from_widget(locator_pixel_region);
        if (vg.has_value()) {
            locator_logic_.render(ClearMode::OFF);
        }
    }
}

void MinimapLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "MinimapLogic\n";
}
