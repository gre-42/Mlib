#include "Preload.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array_Filter.hpp>
#include <Mlib/Geometry/Morphology.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Smoke_Generation/Surface_Contact_Db.hpp>
#include <Mlib/Physics/Smoke_Generation/Surface_Contact_Info.hpp>
#include <Mlib/Render/Batch_Renderers/Particle_Renderer.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Scene_Particles.hpp>
#include <Mlib/Scene_Graph/Interfaces/Particle_Substrate.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Threads/Thread_Top.hpp>

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
    FunctionGuard fg{ "Preloading" };

    if (args.arguments.contains(KnownArgs::resources)) {
        for (const auto &r : args.arguments.at<std::vector<VariableAndHash<std::string>>>(KnownArgs::resources))
        {
            RenderingContextStack::primary_scene_node_resources().preload_single(
                r,
                RenderableResourceFilter{});
        }
    }
    if (args.arguments.contains(KnownArgs::file)) {
        RenderingContextStack::primary_scene_node_resources().preload_many(
            args.arguments.at(KnownArgs::file),
            RenderableResourceFilter{});
    }
    if (args.arguments.contains(KnownArgs::tire_contacts)) {
        for (const auto &r : args.arguments.at<std::vector<VariableAndHash<std::string>>>(KnownArgs::tire_contacts))
        {
            auto res = RenderingContextStack::primary_scene_node_resources().get_arrays(
                r,
                ColoredVertexArrayFilter{
                    .included_tags = PhysicsMaterial::ATTR_COLLIDE
                });
            auto preload_cvas = [&](const auto &cvas) {
                for (const auto &a : cvas) {
                    const SurfaceContactInfo* c = args.surface_contact_db.get_contact_info(
                        a->morphology.physics_material,
                        PhysicsMaterial::SURFACE_BASE_TIRE);
                    if (c != nullptr) {
                        for (const auto& s : c->smoke_infos) {
                            switch (s.particle.substrate) {
                                case ParticleSubstrate::AIR:
                                    air_particles.particle_renderer->preload(s.particle.resource_name);
                                    break;
                                case ParticleSubstrate::SKIDMARK:
                                    skidmark_particles.particle_renderer->preload(s.particle.resource_name);
                                    break;
                                default:
                                    THROW_OR_ABORT("Unknown substrate: " + std::to_string((int)s.particle.substrate));
                            }
                        }
                        // RenderingContextStack::primary_scene_node_resources().preload_single(
                        //     c->smoke_particle_resource_name, RenderableResourceFilter{});
                    }
                }
            };
            preload_cvas(res->scvas);
            preload_cvas(res->dcvas);
        }
    }
}
