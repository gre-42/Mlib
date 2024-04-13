#include "Imposters.hpp"
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Render/Render_Logics/Imposter_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>

using namespace Mlib;

Imposters::Imposters(
    RenderingResources& rendering_resources,
    RenderLogics& render_logics,
    RenderLogic& child_logic,
    Scene& scene,
    SelectedCameras& cameras)
    : rendering_resources_{ rendering_resources }
    , render_logics_{ render_logics }
    , child_logic_{ child_logic }
    , scene_{ scene }
    , cameras_{ cameras }
{}

void Imposters::create_imposter(
    DanglingRef<SceneNode> scene_node,
    const std::string& debug_prefix,
    uint32_t max_texture_size)
{
    auto& imposter_logic = global_object_pool.create<ImposterLogic>(
        CURRENT_SOURCE_LOCATION,
        rendering_resources_,
        child_logic_,
        scene_,
        scene_node,
        cameras_,
        debug_prefix,
        max_texture_size);
    imposter_logic.on_node_clear.add([&imposter_logic]() { global_object_pool.remove(imposter_logic); }, CURRENT_SOURCE_LOCATION);
    render_logics_.prepend(
        { imposter_logic, CURRENT_SOURCE_LOCATION },
        0,                          // z_order
        CURRENT_SOURCE_LOCATION);
}
