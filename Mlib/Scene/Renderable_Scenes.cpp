#include "Renderable_Scenes.hpp"
#include <Mlib/Layout/Layout_Constraint_Parameters.hpp>
#include <Mlib/Layout/Widget.hpp>
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>
#include <Mlib/Scene/Generic_Scenes_Impl.hpp>

using namespace Mlib;

RenderableScenes::RenderableScenes()
    : GenericScenes<RenderableScene>{ "Renderable scenes: "}
{}

RenderableScenes::~RenderableScenes() = default;

std::optional<RenderSetup> RenderableScenes::try_render_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderedSceneDescriptor& frame_id) const
{
    return std::nullopt;
}

void RenderableScenes::render_without_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    std::shared_lock lock{ mutex_ };
    for (auto& n : fullscreen_scenes_) {
        (*this)[n].render_toplevel(
            lx,
            ly,
            render_config,
            scene_graph_config,
            render_results,
            frame_id);
    }
    if (tiled_scenes_.size() <= 1) {
        for (auto& [_, scenes] : tiled_scenes_) {
            for (auto& n : scenes) {
                (*this)[n].render_toplevel(
                    lx,
                    ly,
                    render_config,
                    scene_graph_config,
                    render_results,
                    frame_id);
            }
        }
    } else if (tiled_scenes_.size() == 2) {
        float lx_end = std::floor(lx.end_pixel / 2.f);
        {
            auto region = PixelRegion{ lx.min_pixel, lx_end, ly.min_pixel, ly.end_pixel, RegionRoundMode::DISABLED };
            auto vg = ViewportGuard::from_widget(region);
            for (auto& n : tiled_scenes_.begin()->second) {
                (*this)[n].render_toplevel(
                        LayoutConstraintParameters::child_x(lx, region),
                        ly,
                        render_config,
                        scene_graph_config,
                        render_results,
                        frame_id);
            }
        }
        {
            auto region = PixelRegion{ lx_end, lx.end_pixel, ly.min_pixel, ly.end_pixel, RegionRoundMode::DISABLED };
            auto vg = ViewportGuard::from_widget(region);
            for (auto& n : tiled_scenes_.rbegin()->second) {
                (*this)[n].render_toplevel(
                        LayoutConstraintParameters::child_x(lx, region),
                        ly,
                        render_config,
                        scene_graph_config,
                        render_results,
                        frame_id);
            }
        }
    } else {
        THROW_OR_ABORT("Number of tiled scenes exceeds 2");
    }
}

void RenderableScenes::print(std::ostream& ostr, size_t depth) const
{
    std::shared_lock lock{ mutex_ };
    for (const auto& [n, rs] : guarded_iterable()) {
        ostr << std::string(depth, ' ') << n << '\n';
        rs.print(ostr, depth + 1);
    }
}

void RenderableScenes::add_fullscreen_scene(std::string scene)
{
    std::scoped_lock lock{ mutex_ };
    fullscreen_scenes_.emplace_back(std::move(scene));
}

void RenderableScenes::add_tiled_scene(
    uint32_t tile,
    std::string scene)
{
    std::scoped_lock lock{ mutex_ };
    tiled_scenes_[tile].emplace_back(std::move(scene));
}

template GuardedIterable<GenericScenes<RenderableScene>::map_type::iterator, std::shared_lock<SafeAtomicRecursiveSharedMutex>>
    GenericScenes<RenderableScene>::guarded_iterable();
template GuardedIterable<GenericScenes<RenderableScene>::map_type::const_iterator, std::shared_lock<SafeAtomicRecursiveSharedMutex>>
    GenericScenes<RenderableScene>::guarded_iterable() const;
template RenderableScene* GenericScenes<RenderableScene>::try_get(const std::string& name);
template const RenderableScene* GenericScenes<RenderableScene>::try_get(const std::string& name) const;
