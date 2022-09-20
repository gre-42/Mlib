#include "Impostors.hpp"
#include <Mlib/Render/Render_Logics/Impostor_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>

using namespace Mlib;

Impostors::Impostors(
    RenderLogics& render_logics,
    RenderLogic& child_logic,
    Scene& scene,
    SelectedCameras& cameras)
: render_logics_{render_logics},
  child_logic_{child_logic},
  scene_{scene},
  cameras_{cameras}
{}

void Impostors::create_impostor(SceneNode& scene_node) {
    render_logics_.prepend(&scene_node, std::make_shared<ImposterLogic>(
        child_logic_,
        scene_,
        scene_node,
        cameras_));
}
