#include "Preload.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Smoke_Generation/Surface_Contact_Db.hpp>
#include <Mlib/Physics/Smoke_Generation/Surface_Contact_Info.hpp>
#include <Mlib/Render/Batch_Renderers/Particles_Instance.hpp>
#include <Mlib/Render/Particle_Resources.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <filesystem>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(resources);
DECLARE_ARGUMENT(file);
DECLARE_ARGUMENT(tire_contacts);
}

const std::string Preload::key = "preload";

LoadSceneJsonUserFunction Preload::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    Preload(args.renderable_scene()).execute(args);
};

Preload::Preload(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void Preload::execute(const LoadSceneJsonUserFunctionArgs &args) {
    if (args.arguments.contains(KnownArgs::resources)) {
        for (const auto &r : args.arguments.at<std::vector<std::string>>(KnownArgs::resources))
        {
            primary_rendering_context.scene_node_resources.preload_single(
                r,
                RenderableResourceFilter{});
        }
    }
    if (args.arguments.contains(KnownArgs::file)) {
        primary_rendering_context.scene_node_resources.preload_many(
            args.arguments.at(KnownArgs::file),
            RenderableResourceFilter{});
    }
    if (args.arguments.contains(KnownArgs::tire_contacts)) {
        for (const auto &r : args.arguments.at<std::vector<std::string>>(KnownArgs::tire_contacts))
        {
            auto res = primary_rendering_context.scene_node_resources.get_physics_arrays(r);
            auto preload_cvas = [&](const auto &cvas) {
                for (const auto &a : cvas) {
                    if (!any(a->physics_material & PhysicsMaterial::ATTR_COLLIDE)) {
                        continue;
                    }
                    auto c = args.surface_contact_db.get_contact_info(
                        a->physics_material,
                        PhysicsMaterial::SURFACE_BASE_TIRE);
                    if (c != nullptr) {
                        particle_resources.preload_instantiator(c->smoke_particle_resource_name);
                        // primary_rendering_context.scene_node_resources.preload_single(
                        //     c->smoke_particle_resource_name, RenderableResourceFilter{});
                    }
                }
            };
            preload_cvas(res->scvas);
            preload_cvas(res->dcvas);
        }
    }
}
