#include "Preload.hpp"
#include <Mlib/Geometry/Material/Particle_Type.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array_Filter.hpp>
#include <Mlib/Geometry/Morphology.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/OpenGL/Batch_Renderers/Particle_Renderer.hpp>
#include <Mlib/Os/Threads/Thread_Top.hpp>
#include <Mlib/Physics/Smoke_Generation/Surface_Contact_Db.hpp>
#include <Mlib/Physics/Smoke_Generation/Surface_Contact_Info.hpp>
#include <Mlib/Resource_Context/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene/Scene_Particles.hpp>
#include <Mlib/Scene_Graph/Interfaces/ITrail_Renderer.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Resources/Resource_Does_Not_Exist_Behavior.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#ifndef WITHOUT_AUDIO
#include <Mlib/Audio/Audio_Resource_Context.hpp>
#include <Mlib/Audio/Audio_Resources.hpp>
#endif

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(files);
DECLARE_ARGUMENT(audio_buffers);
DECLARE_ARGUMENT(audio_buffer_sequences);
DECLARE_ARGUMENT(resources);
DECLARE_ARGUMENT(tire_contacts);
DECLARE_ARGUMENT(trails);
DECLARE_ARGUMENT(throw_if_file_resource_unknown);
}

namespace FilesKnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(audio_buffers);
DECLARE_ARGUMENT(audio_buffer_sequences);
DECLARE_ARGUMENT(resources);
DECLARE_ARGUMENT(tire_contacts);
DECLARE_ARGUMENT(trails);
}

std::list<std::vector<VariableAndHash<std::string>>> get_names(
    const JsonMacroArguments& variables,
    const std::optional<JsonMacroArguments>& files,
    std::string_view attr)
{
    std::list<std::vector<VariableAndHash<std::string>>> result;
    if (variables.contains(attr)) {
        result.push_back(variables.at<std::vector<VariableAndHash<std::string>>>(attr));
    }
    if (files.has_value() && files->contains(attr)) {
        auto filename = files->path(attr);
        auto fstr = create_ifstream(filename);
        if (fstr->fail()) {
            throw std::runtime_error("Could not open preload-file for read: \"" + filename.string() + '"');
        }
        nlohmann::json j;
        *fstr >> j;
        if (fstr->fail()) {
            throw std::runtime_error("Could not load from file: \"" + filename.string() + '"');
        }
        std::vector<VariableAndHash<std::string>> resource_names;
        try {
            resource_names = j.get<std::vector<VariableAndHash<std::string>>>();
        } catch (const nlohmann::json::parse_error&) {
            throw std::runtime_error("Could not parse file: \"" + filename.string() + '"');
        } catch (const nlohmann::json::type_error&) {
            throw std::runtime_error("Could not parse file: \"" + filename.string() + '"');
        }
        result.push_back(resource_names);
    }
    return result;
}

Preload::Preload(PhysicsScene& physics_scene)
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void Preload::execute(const LoadSceneJsonUserFunctionArgs& args) {
    FunctionGuard fg{ "Preloading" };

    args.arguments.validate(KnownArgs::options);

    auto files = args.arguments.try_get_child(KnownArgs::files);
    if (files.has_value()) {
        files->validate(FilesKnownArgs::options);
    }

    auto e = args.arguments.at<bool>(KnownArgs::throw_if_file_resource_unknown, true)
        ? PreloadResourceDoesNotExistBehavior::THROW
        : PreloadResourceDoesNotExistBehavior::IGNORE;

#ifndef WITHOUT_AUDIO
    for (const auto& vec : get_names(args.arguments, files, KnownArgs::audio_buffers)) {
        for (const auto& r : vec) {
            AudioResourceContextStack::primary_resource_context().audio_resources->preload_buffer(r, e);
        }
    }

    for (const auto& vec : get_names(args.arguments, files, KnownArgs::audio_buffer_sequences)) {
        for (const auto& r : vec) {
            AudioResourceContextStack::primary_resource_context().audio_resources->preload_buffer_sequence(r, e);
        }
    }
#endif

    for (const auto& vec : get_names(args.arguments, files, KnownArgs::resources)) {
        for (const auto& r : vec) {
            RenderingContextStack::primary_scene_node_resources().preload_single(
                r,
                RenderableResourceFilter{},
                e);
        }
    }

    for (const auto& vec : get_names(args.arguments, files, KnownArgs::tire_contacts)) {
        for (const auto& r : vec) {
            auto res = RenderingContextStack::primary_scene_node_resources().get_arrays(
                r,
                ColoredVertexArrayFilter{
                    .included_tags = PhysicsMaterial::ATTR_COLLIDE
                });
            auto preload_cvas = [&](const auto &cvas) {
                for (const auto &a : cvas) {
                    for (auto material1 : {
                        PhysicsMaterial::SURFACE_BASE_TIRE,
                        PhysicsMaterial::SURFACE_BASE_FOOT})
                    {
                        const SurfaceContactInfo* c = args.surface_contact_db.get_contact_info(
                            a->meta.morphology.physics_material,
                            material1);
                        if (c != nullptr) {
                            for (const auto& s : c->emission) {
                                if (s.visual.has_value()) {
                                    switch (s.visual->particle.type) {
                                        case ParticleType::SMOKE:
                                            air_particles.particle_renderer->preload(s.visual->particle.resource_name);
                                            break;
                                        case ParticleType::SKIDMARK:
                                            skidmark_particles.particle_renderer->preload(s.visual->particle.resource_name);
                                            break;
                                        case ParticleType::WATER_WAVE:
                                            throw std::runtime_error("Water waves do not require particle preloading");
                                        case ParticleType::SEA_SPRAY:
                                            sea_spray_particles.particle_renderer->preload(s.visual->particle.resource_name);
                                            break;
                                        default:
                                            throw std::runtime_error("Unknown particle type (2): " + std::to_string((uint32_t)s.visual->particle.type));
                                    }
                                }
                                #ifndef WITHOUT_AUDIO
                                if (s.audio != nullptr) {
                                    s.audio->preload();
                                }
                                #endif
                            }
                            // RenderingContextStack::primary_scene_node_resources().preload_single(
                            //     c->smoke_particle_resource_name, RenderableResourceFilter{});
                        }
                    }
                }
            };
            preload_cvas(res->scvas);
            preload_cvas(res->dcvas);
        }
    }
    for (const auto& vec : get_names(args.arguments, files, KnownArgs::trails)) {
        for (const auto& r : vec) {
            trail_renderer.preload(r);
        }
    }
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "preload",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                Preload(args.physics_scene()).execute(args);
            });
    }
} obj;

}
