#include "Set_Particle_Renderer.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Material/Particle_Type.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Render/Batch_Renderers/Particle_Renderer.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(renderable);
}

SetParticleRenderer::SetParticleRenderer(PhysicsScene& physics_scene) 
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void SetParticleRenderer::execute(const LoadSceneJsonUserFunctionArgs& args) const
{
    args.arguments.validate(KnownArgs::options);
    (*this)(
        args.arguments.at<VariableAndHash<std::string>>(KnownArgs::node),
        args.arguments.at<VariableAndHash<std::string>>(KnownArgs::renderable));
}

void SetParticleRenderer::operator () (
    const VariableAndHash<std::string>& node,
    const VariableAndHash<std::string>& renderable) const
{
    auto particle_renderer = std::make_shared<ParticleRenderer>(
        particle_resources,
        ParticleType::SMOKE);
    scene.get_node(node, DP_LOC)->set_particle_renderer(renderable, particle_renderer);
    physics_engine.advance_times_.add_advance_time(
        DanglingBaseClassRef<ParticleRenderer>{
            *particle_renderer,
            CURRENT_SOURCE_LOCATION},
        CURRENT_SOURCE_LOCATION);
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "set_particle_renderer",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                SetParticleRenderer(args.physics_scene()).execute(args);
            });
    }
} obj;

}
