#include "Cleanup_Mesh.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Cleanup/Merge_Neighboring_Points.hpp>
#include <Mlib/Geometry/Mesh/Cleanup/Modulo_Uv.hpp>
#include <Mlib/Geometry/Mesh/Cleanup/Remove_Degenerate_Triangles.hpp>
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
DECLARE_ARGUMENT(min_vertex_distance);
DECLARE_ARGUMENT(min_distance_material_filter);
DECLARE_ARGUMENT(modulo_uv);
}

const std::string CleanupMesh::key = "cleanup_mesh";

LoadSceneJsonUserFunction CleanupMesh::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);

    auto min_distance_filter = PhysicsMaterial::NONE;
    auto min_vertex_distance = args.arguments.at<float>(KnownArgs::min_vertex_distance, 0.f);
    if (min_vertex_distance != 0.f) {
        min_distance_filter = physics_material_from_string(args.arguments.at<std::string>(KnownArgs::min_distance_material_filter));
    }
    RenderingContextStack::primary_scene_node_resources().add_modifier(
        args.arguments.at<std::string>(KnownArgs::resource_name),
        [min_distance_filter,
         min_vertex_distance,
         modulo_uv = args.arguments.at<bool>(KnownArgs::modulo_uv, true)]
        (ISceneNodeResource& resource)
        {
            auto cleanup = [min_distance_filter, min_vertex_distance, modulo_uv]<class TPos>(
                Bvh<TPos, FixedArray<TPos, 3>, 3>& bvh,
                std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& cvas)
            {
                cvas.remove_if([&bvh, min_distance_filter, min_vertex_distance, modulo_uv](auto& cva){
                    if ((min_vertex_distance != 0) &&
                        any(cva->physics_material & min_distance_filter))
                    {
                        merge_neighboring_points<TPos>(*cva, bvh, min_vertex_distance);
                    }
                    remove_degenerate_triangles(*cva);
                    // remove_triangles_with_opposing_normals(*cva);
                    if (modulo_uv) {
                        Mlib::modulo_uv(*cva);
                    }
                    return cva->triangles.empty();
                });
            };
            Bvh<float, FixedArray<float, 3>, 3> sbvh{FixedArray<float, 3>{0.1f, 0.1f, 0.1f}, 17};
            Bvh<double, FixedArray<double, 3>, 3> dbvh{FixedArray<double, 3>{0.1, 0.1, 0.1}, 17};
            for (const auto& acva : resource.get_rendering_arrays()) {
                cleanup(sbvh, acva->scvas);
                cleanup(dbvh, acva->dcvas);
            }
        });
};
