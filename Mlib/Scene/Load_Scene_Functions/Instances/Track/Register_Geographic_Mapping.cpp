#include "Register_Geographic_Mapping.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix_Json.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(transformation);
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
    auto absolute_model_matrix = transformation_matrix_from_json<float, ScenePos, 3>(
        args.arguments.at(KnownArgs::transformation));

    scene_node_resources.register_geographic_mapping(
        args.arguments.at<std::string>(KnownArgs::resource),
        args.arguments.at<std::string>(KnownArgs::name),
        absolute_model_matrix.casted<double, double>());
}
