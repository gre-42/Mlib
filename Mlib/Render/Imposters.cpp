#include "Imposters.hpp"
#include <Mlib/Render/Render_Logics/Imposter_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>

using namespace Mlib;

Imposters::Imposters(
    RenderLogics& render_logics,
    RenderLogic& child_logic,
    Scene& scene,
    SelectedCameras& cameras)
: render_logics_{render_logics},
  child_logic_{child_logic},
  scene_{scene},
  cameras_{cameras}
{}

void Imposters::create_imposter(
    SceneNode& scene_node,
    const std::string& debug_prefix,
    uint32_t max_texture_size)
{
    render_logics_.prepend(&scene_node, std::make_shared<ImposterLogic>(
        child_logic_,
        scene_,
        scene_node,
        cameras_,
        debug_prefix,
        max_texture_size));
}
