#include <Mlib/Geometry/Mesh/Set_Bounds.hpp>
#include <Mlib/Geometry/Primitives/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Primitives/Bounding_Sphere.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Resource_Context/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;

namespace KnownAabbArgs {
    BEGIN_ARGUMENT_LIST;
    DECLARE_ARGUMENT(min);
    DECLARE_ARGUMENT(max);
}

struct LoadableAabb3f : public AxisAlignedBoundingBox<CompressedScenePos, 3> {
    LoadableAabb3f() : AxisAlignedBoundingBox{ uninitialized } {}
    LoadableAabb3f& operator = (const AxisAlignedBoundingBox<CompressedScenePos, 3>& other) {
        AxisAlignedBoundingBox<CompressedScenePos, 3>& base = *this;
        base = other;
        return *this;
    }
};

struct LoadableBoundingSphere3f : public BoundingSphere<CompressedScenePos, 3> {
    LoadableBoundingSphere3f() : BoundingSphere<CompressedScenePos, 3>{ uninitialized } {}
    LoadableBoundingSphere3f& operator = (const BoundingSphere<CompressedScenePos, 3>& other) {
        BoundingSphere<CompressedScenePos, 3>& base = *this;
        base = other;
        return *this;
    }
};

static void from_json(const nlohmann::json& j, LoadableAabb3f& aabb) {
    JsonView jv{ j };
    jv.validate(KnownAabbArgs::options);
    aabb = AxisAlignedBoundingBox<CompressedScenePos, 3>::from_min_max(
        jv.at<EFixedArray<CompressedScenePos, 3>>(KnownAabbArgs::min),
        jv.at<EFixedArray<CompressedScenePos, 3>>(KnownAabbArgs::max));
}

namespace KnownSphereArgs {
    BEGIN_ARGUMENT_LIST;
    DECLARE_ARGUMENT(center);
    DECLARE_ARGUMENT(radius);
}

static void from_json(const nlohmann::json& j, LoadableBoundingSphere3f& sphere) {
    JsonView jv{ j };
    jv.validate(KnownSphereArgs::options);
    sphere = BoundingSphere<CompressedScenePos, 3>{
        jv.at<EFixedArray<CompressedScenePos, 3>>(KnownSphereArgs::center),
        jv.at<CompressedScenePos>(KnownSphereArgs::radius) };
}

namespace {

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(resource);
DECLARE_ARGUMENT(aabb);
DECLARE_ARGUMENT(sphere);
}

static void execute(const LoadSceneJsonUserFunctionArgs& args)
{
    RenderingContextStack::primary_scene_node_resources().add_modifier(
        args.arguments.at<VariableAndHash<std::string>>(KnownArgs::resource),
        [aabb = args.arguments.at<LoadableAabb3f>(KnownArgs::aabb),
         sphere = args.arguments.at<LoadableBoundingSphere3f>(KnownArgs::sphere)]
        (ISceneNodeResource& resource) {
            for (auto& l : resource.get_rendering_arrays()) {
                set_bounds(*l, aabb, sphere);
            }
        });
}

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "set_bounds",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
                execute(args);
            });
    }
} obj;

}
