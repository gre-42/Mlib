#include "Create_Skidmark.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Render_Logics/Skidmark_Logic.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene/Scene_Particles.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Elements/Skidmark.hpp>
#include <Mlib/Scene_Graph/Interfaces/Particle_Type.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

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
    auto node = scene.get_node(node_name, DP_LOC);
    auto particle_type = particle_type_from_string(args.arguments.at<std::string>(KnownArgs::particle_type));
    auto skidmark = std::make_shared<Skidmark>(Skidmark{
        .particle_type = particle_type,
        .texture = nullptr,
        .vp = fixed_nans<ScenePos, 4, 4>()
        });
    auto& particle_renderer = [&]() -> auto& {
        switch (particle_type) {
        case ParticleType::SMOKE:
            THROW_OR_ABORT("Smoke particles do not require a skidmark logic");
        case ParticleType::SKIDMARK:
            return *skidmark_particles.particle_renderer;
        case ParticleType::WATER_WAVE:
            THROW_OR_ABORT("Water waves do not require a skidmark logic");
        case ParticleType::SEA_SPRAY:
            return *sea_spray_particles.particle_renderer;
        }
        THROW_OR_ABORT("Unknown particle type: " + std::to_string((int)particle_type));
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
                args.arguments.validate(KnownArgs::options);
                CreateSkidmark(args.renderable_scene()).execute(args);
            });
    }
} obj;

}
