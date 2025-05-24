#include "Deferred_Instantiator.hpp"
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Render/Render_Logics/Imposter_Logic.hpp>
#include <Mlib/Render/Render_Logics/Post_Processing_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Render_Logics/Standard_Render_Logic.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

DeferredInstantiator::DeferredInstantiator()
    : imposters_created_{ false }
{}

DeferredInstantiator::~DeferredInstantiator() = default;

void DeferredInstantiator::set_imposter_info(
    const DanglingRef<SceneNode>& scene_node,
    const ImposterInfo& info)
{
    if (imposters_created_) {
        THROW_OR_ABORT("DeferredInstantiator already created");
    }
    if (!infos_.try_emplace(scene_node.ptr(), info).second) {
        THROW_OR_ABORT("Imposter info already set");
    }
}

void DeferredInstantiator::create_imposters(
    IRenderableScene* renderable_scene,
    RenderingResources& rendering_resources,
    RenderLogics& render_logics,
    RenderLogic& child_logic,
    Scene& scene,
    SelectedCameras& cameras) const
{
    for (const auto& [scene_node, info] : infos_) {
        auto& imposter_logic = global_object_pool.create<ImposterLogic>(
            CURRENT_SOURCE_LOCATION,
            renderable_scene,
            rendering_resources,
            child_logic,
            scene,
            *scene_node,
            cameras,
            info.debug_prefix,
            info.max_texture_size);
        imposter_logic.on_node_clear.add([&imposter_logic]() { global_object_pool.remove(imposter_logic); }, CURRENT_SOURCE_LOCATION);
        render_logics.prepend(
            { imposter_logic, CURRENT_SOURCE_LOCATION },
            0,                          // z_order
            CURRENT_SOURCE_LOCATION);
    }
}

void DeferredInstantiator::set_background_color(
    const FixedArray<float, 3>& background_color)
{
    background_color_ = background_color;
}

void DeferredInstantiator::apply_background_color(
    StandardRenderLogic& standard_render_logic,
    PostProcessingLogic& post_processing_logic)
{
    if (!background_color_.has_value()) {
        THROW_OR_ABORT("Background color not set");
    }
    standard_render_logic.set_background_color(*background_color_);
    post_processing_logic.set_background_color(*background_color_);
}
