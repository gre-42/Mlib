#include "Create_Skidmark.hpp"
#include <Mlib/Geometry/Material/Particle_Type.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/OpenGL/Render_Logics/Render_Logics.hpp>
#include <Mlib/OpenGL/Render_Logics/Skidmark_Logic.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene/Scene_Particles.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Elements/Skidmark.hpp>
#include <stdexcept>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(texture_width);
DECLARE_ARGUMENT(texture_height);
DECLARE_ARGUMENT(particle_type);
}

CreateSkidmark::CreateSkidmark(RenderableScene& renderable_scene) 
    : LoadRenderableSceneInstanceFunction{ renderable_scene }
{}

void CreateSkidmark::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    auto node_name = args.arguments.at<VariableAndHash<std::string>>(KnownArgs::node);
    auto node = scene.get_node(node_name, CURRENT_SOURCE_LOCATION);
    auto particle_type = particle_type_from_string(args.arguments.at<std::string>(KnownArgs::particle_type));
    auto skidmark = std::make_shared<Skidmark>(Skidmark{
        .particle_type = particle_type,
        .texture = nullptr,
        .vp = fixed_nans<ScenePos, 4, 4>()
        });
    auto& particle_renderer = [&]() -> auto& {
        switch (particle_type) {
        case ParticleType::NONE:
            throw std::runtime_error("Particle type \"none\" does not require a skidmark logic");
        case ParticleType::SMOKE:
            throw std::runtime_error("Smoke particles do not require a skidmark logic");
        case ParticleType::SKIDMARK:
            return *skidmark_particles.particle_renderer;
        case ParticleType::WATER_WAVE:
            throw std::runtime_error("Water waves do not require a skidmark logic");
        case ParticleType::SEA_SPRAY:
            return *sea_spray_particles.particle_renderer;
        }
        throw std::runtime_error("Unknown particle type: " + std::to_string((int)particle_type));
    }();
    auto o = object_pool.create_unique<SkidmarkLogic>(
        CURRENT_SOURCE_LOCATION,
        node,
        skidmark,
        particle_renderer,
        args.arguments.at<int>(KnownArgs::texture_width),
        args.arguments.at<int>(KnownArgs::texture_height));
    o->on_skidmark_node_clear.add([&p=object_pool, &o=*o](){ p.remove(o); }, CURRENT_SOURCE_LOCATION);
    render_logics.prepend(
        { *o, CURRENT_SOURCE_LOCATION },
        0 /* z_order */,
        CURRENT_SOURCE_LOCATION);
    node->add_skidmark(skidmark);
    o.release();
}


namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "create_skidmark",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                CreateSkidmark(args.renderable_scene()).execute(args);
            });
    }
} obj;

}
