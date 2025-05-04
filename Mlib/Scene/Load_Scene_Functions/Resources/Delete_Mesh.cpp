#include "Delete_Mesh.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(resource_name);
DECLARE_ARGUMENT(where);
}

const std::string DeleteMesh::key = "delete_mesh";

LoadSceneJsonUserFunction DeleteMesh::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);

    auto where = physics_material_from_string(args.arguments.at<std::string>(KnownArgs::where));

    RenderingContextStack::primary_scene_node_resources().add_modifier(
        args.arguments.at<VariableAndHash<std::string>>(KnownArgs::resource_name),
        [where](ISceneNodeResource& resource)
        {
            auto del = [where]<class TPos>(
                std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& cvas)
            {
                cvas.remove_if([where](auto& cva){
                    return any(cva->morphology.physics_material & where);
                });
            };
            for (const auto& acva : resource.get_rendering_arrays()) {
                del(acva->scvas);
                del(acva->dcvas);
            }
        });
};
