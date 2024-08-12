#include "Register_Geographic_Mapping.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Physics/Units.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(position);
DECLARE_ARGUMENT(rotation);
DECLARE_ARGUMENT(scale);
DECLARE_ARGUMENT(resource);
}

const std::string RegisterGeographicMapping::key = "register_geographic_mapping";

LoadSceneJsonUserFunction RegisterGeographicMapping::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    RegisterGeographicMapping(args.renderable_scene()).execute(args);
};

RegisterGeographicMapping::RegisterGeographicMapping(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void RegisterGeographicMapping::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    TransformationMatrix<float, ScenePos, 3> absolute_model_matrix{
        tait_bryan_angles_2_matrix(args.arguments.at<UFixedArray<float, 3>>(KnownArgs::rotation) * degrees) *
        args.arguments.at<float>(KnownArgs::scale),
        args.arguments.at<UFixedArray<ScenePos, 3>>(KnownArgs::position) * (ScenePos)meters};

    scene_node_resources.register_geographic_mapping(
        args.arguments.at<std::string>(KnownArgs::resource),
        args.arguments.at<std::string>(KnownArgs::name),
        absolute_model_matrix.casted<double, double>());
}
