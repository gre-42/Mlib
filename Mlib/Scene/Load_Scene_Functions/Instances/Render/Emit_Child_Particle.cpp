#include "Emit_Child_Particle.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix_Json.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Interfaces/IParticle_Creator.hpp>
#include <Mlib/Scene_Graph/Interfaces/IParticle_Renderer.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(resource);
DECLARE_ARGUMENT(location);
}

EmitChildParticle::EmitChildParticle(PhysicsScene& physics_scene) 
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void EmitChildParticle::execute(const LoadSceneJsonUserFunctionArgs& args) const
{
    args.arguments.validate(KnownArgs::options);
    (*this)(
        args.arguments.at<VariableAndHash<std::string>>(KnownArgs::node),
        args.arguments.at<VariableAndHash<std::string>>(KnownArgs::resource),
        transformation_matrix_from_json<SceneDir, ScenePos, 3>(args.arguments.at(KnownArgs::location)));
}

void EmitChildParticle::operator () (
    const VariableAndHash<std::string>& node,
    const VariableAndHash<std::string>& resource,
    const TransformationMatrix<SceneDir, ScenePos, 3>& location) const
{
    scene.get_node(node, DP_LOC)->
        get_particle_renderer()->
        get_instantiator(resource).add_particle(
            location,
            fixed_zeros<float, 3>(),    // velocity
            0.f,                        // air_resistance
            0.f);                       // texture_layer
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "emit_child_particle",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                EmitChildParticle(args.physics_scene()).execute(args);
            });
    }
} obj;

}
