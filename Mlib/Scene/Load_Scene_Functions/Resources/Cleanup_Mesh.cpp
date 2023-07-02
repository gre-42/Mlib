#include "Cleanup_Mesh.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Cleanup/Remove_Degenerate_Triangles.hpp>
#include <Mlib/Geometry/Mesh/Cleanup/Remove_Triangles_With_Opposing_Normals.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(resource_name);
}

const std::string CleanupMesh::key = "cleanup_mesh";

LoadSceneJsonUserFunction CleanupMesh::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);

    RenderingContextStack::primary_scene_node_resources().add_modifier(
        args.arguments.at<std::string>(KnownArgs::resource_name),
        [](ISceneNodeResource& resource)
        {
            for (auto acva : resource.get_rendering_arrays()) {
                acva->scvas.remove_if([](auto& cva){
                    remove_degenerate_triangles(*cva);
                    remove_triangles_with_opposing_normals(*cva);
                    return cva->triangles.empty();
                });
                acva->dcvas.remove_if([](auto& cva){
                    remove_degenerate_triangles(*cva);
                    remove_triangles_with_opposing_normals(*cva);
                    return cva->triangles.empty();
                });
            }
        });
};
