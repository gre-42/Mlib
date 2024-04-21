#include "Make_Triangles_With_Opposing_Normals_Two_Sided.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Modifiers/Make_Triangles_With_Opposing_Normals_Two_Sided.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(resource_name);
DECLARE_ARGUMENT(material_filter);
}

const std::string MakeTrianglesWithOpposingNormalsTwoSided::key = "make_triangles_with_opposing_normals_two_sided";

LoadSceneJsonUserFunction MakeTrianglesWithOpposingNormalsTwoSided::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);

    auto material_filter = PhysicsMaterial::NONE;
    if (auto f = args.arguments.try_at<std::string>(KnownArgs::material_filter); f.has_value()) {
        material_filter = physics_material_from_string(f.value());
    }
    make_triangles_with_opposing_normals_two_sided(
        args.arguments.at<std::string>(KnownArgs::resource_name),
        RenderingContextStack::primary_scene_node_resources(),
        material_filter);
};
