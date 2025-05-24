#include "Create_Rigid_Statics.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Env.hpp>
#include <Mlib/Geometry/Exceptions/Polygon_Edge_Exception.hpp>
#include <Mlib/Geometry/Exceptions/Triangle_Exception.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array_Filter.hpp>
#include <Mlib/Geometry/Mesh/Save_Polygon_To_Obj.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix_Json.hpp>
#include <Mlib/Physics/Collision/Collidable_Mode.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle_Flags.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Primitives.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Absolute_Movable_Setter.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Strings/String.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(transformation);
DECLARE_ARGUMENT(hitboxes);
DECLARE_ARGUMENT(v);
DECLARE_ARGUMENT(w);
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(asset_id);
DECLARE_ARGUMENT(included_names);
DECLARE_ARGUMENT(excluded_names);
DECLARE_ARGUMENT(flags);
}

CreateRigidStatics::CreateRigidStatics(PhysicsScene& physics_scene) 
: LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void CreateRigidStatics::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto absolute_model_matrix = transformation_matrix_from_json<float, ScenePos, 3>(
        args.arguments.at(KnownArgs::transformation));

    auto rb = rigid_cuboid(
        object_pool,
        args.arguments.at<std::string>(KnownArgs::name),
        args.arguments.at<std::string>(KnownArgs::asset_id),
        INFINITY * kg,
        fixed_ones<float, 3>() * meters,    // size
        fixed_zeros<float, 3>() * meters,   // com
        args.arguments.at<UFixedArray<float, 3>>(KnownArgs::v, fixed_zeros<float, 3>()) * kph,
        args.arguments.at<UFixedArray<float, 3>>(KnownArgs::w, fixed_zeros<float, 3>()) * rpm,
        fixed_zeros<float, 3>() * degrees,  // I_rotation
        nullptr,                            // pl
        scene_node_resources.get_geographic_mapping(VariableAndHash<std::string>{"world"}));
    rb->set_absolute_model_matrix(absolute_model_matrix);

    if (args.arguments.contains(KnownArgs::flags)) {
        rb->flags_ = rigid_body_vehicle_flags_from_string(args.arguments.at<std::string>(KnownArgs::flags));
    }
    std::list<std::shared_ptr<ColoredVertexArray<float>>> s_hitboxes;
    std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>> d_hitboxes;
    if (args.arguments.contains_non_null(KnownArgs::hitboxes)) {
        ColoredVertexArrayFilter filter{
            .included_tags = PhysicsMaterial::ATTR_COLLIDE,
            .included_names = Mlib::compile_regex(args.arguments.at<std::string>(KnownArgs::included_names, "")),
            .excluded_names = Mlib::compile_regex(args.arguments.at<std::string>(KnownArgs::excluded_names, "$ ^"))};
        auto acva = scene_node_resources.get_arrays(
            args.arguments.at<VariableAndHash<std::string>>(KnownArgs::hitboxes),
            filter);
        auto insert = [](auto& hitboxes, const auto& cvas){
            for (const auto& cva: cvas) {
                hitboxes.push_back(cva);
            }
        };
        insert(s_hitboxes, acva->scvas);
        insert(d_hitboxes, acva->dcvas);
    }
    try {
        physics_engine.rigid_bodies_.add_rigid_body(
            *rb,
            s_hitboxes,
            d_hitboxes,
            {},
            CollidableMode::STATIC);
        rb.release();
    } catch (const TriangleException<double>& e) {
        if (auto filename = try_getenv("RIGID_BODY_TRIANGLE_FILENAME"); filename.has_value()) {
            save_triangle_to_obj(*filename, {e.a, e.b, e.c});
        }
        throw std::runtime_error(e.str("Error", scene_node_resources.get_geographic_mapping(VariableAndHash<std::string>{"world"})));
    } catch (const PolygonEdgeException<double, 3>& e) {
        if (auto filename = try_getenv("RIGID_BODY_TRIANGLE_FILENAME"); filename.has_value()) {
            save_triangle_to_obj(*filename, e.poly);
        }
        throw std::runtime_error(e.str("Error", scene_node_resources.get_geographic_mapping(VariableAndHash<std::string>{"world"})));
    } catch (const PolygonEdgeException<double, 4>& e) {
        if (auto filename = try_getenv("RIGID_BODY_TRIANGLE_FILENAME"); filename.has_value()) {
            save_quad_to_obj(*filename, e.poly);
        }
        throw std::runtime_error(e.str("Error", scene_node_resources.get_geographic_mapping(VariableAndHash<std::string>{"world"})));
    }
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "rigid_statics",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
                CreateRigidStatics(args.physics_scene()).execute(args);
            });
    }
} obj;

}
