#include "Add_Sphere_Swept_Aapp.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Geometry/Intersection/Intersectors/Swept_Sphere_Aabb.hpp>
#include <Mlib/Geometry/Mesh/Typed_Mesh.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Intersectable_Resource.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(min);
DECLARE_ARGUMENT(max);
DECLARE_ARGUMENT(radius);
DECLARE_ARGUMENT(physics_material);
}

const std::string AddSweptSphereAabb::key = "add_swept_sphere_aabb";

LoadSceneJsonUserFunction AddSweptSphereAabb::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    execute(args);
};

void AddSweptSphereAabb::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto material = physics_material_from_string(args.arguments.at<std::string>(KnownArgs::physics_material));
    auto ssaabb = std::make_shared<SweptSphereAabb<float>>(
        args.arguments.at<UFixedArray<float, 3>>(KnownArgs::min) * meters,
        args.arguments.at<UFixedArray<float, 3>>(KnownArgs::max) * meters,
        args.arguments.at<float>(KnownArgs::radius) * meters);
    auto l = std::list{ TypedMesh<std::shared_ptr<IIntersectable<float>>>{material, ssaabb} };
    auto res = std::make_shared<IntersectableResource>(std::move(l));
    auto& scene_node_resources = RenderingContextStack::primary_scene_node_resources();
    scene_node_resources.add_resource(args.arguments.at<std::string>(KnownArgs::name), res);
}