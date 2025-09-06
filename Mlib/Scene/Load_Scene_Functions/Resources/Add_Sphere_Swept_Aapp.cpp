#include <Mlib/Argument_List.hpp>
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Geometry/Intersection/Intersectors/Swept_Sphere_Aabb.hpp>
#include <Mlib/Geometry/Mesh/Typed_Mesh.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
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

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "add_swept_sphere_aabb",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
                auto material = physics_material_from_string(args.arguments.at<std::string>(KnownArgs::physics_material));
                auto ssaabb = std::make_shared<SweptSphereAabb>(
                    args.arguments.at<EFixedArray<CompressedScenePos, 3>>(KnownArgs::min) * meters,
                    args.arguments.at<EFixedArray<CompressedScenePos, 3>>(KnownArgs::max) * meters,
                    args.arguments.at<CompressedScenePos>(KnownArgs::radius) * meters);
                // Using loader instead so "write_loaded_resources" works as expected.
                RenderingContextStack::primary_scene_node_resources().add_resource_loader(
                    args.arguments.at<VariableAndHash<std::string>>(KnownArgs::name),
                    [l = std::list{ TypedMesh<std::shared_ptr<IIntersectable>>{material, ssaabb} }]()mutable{
                        return std::make_shared<IntersectableResource>(std::move(l));
                    });
            });
    }
} obj;

}
