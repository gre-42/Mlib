#include "Create_Fluid_Subdomain.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box_Json.hpp>
#include <Mlib/Geometry/Intersection/Interval_Json.hpp>
#include <Mlib/Json/Chrono.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Fluid_Subdomain_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene/Scene_Particles.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Elements/Skidmark.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(texture_width);
DECLARE_ARGUMENT(texture_height);
DECLARE_ARGUMENT(directional_velocity);
DECLARE_ARGUMENT(radial_velocity);
DECLARE_ARGUMENT(velocity_region);
DECLARE_ARGUMENT(angular_velocity);
DECLARE_ARGUMENT(velocity_dt);
DECLARE_ARGUMENT(speed_of_sound);
DECLARE_ARGUMENT(time_relaxation_constant);
DECLARE_ARGUMENT(density_normalization);
DECLARE_ARGUMENT(reference_inner_velocity);
DECLARE_ARGUMENT(maximum_inner_velocity);
DECLARE_ARGUMENT(visual_density);
}

CreateFluidSubdomain::CreateFluidSubdomain(RenderableScene& renderable_scene) 
    : LoadRenderableSceneInstanceFunction{ renderable_scene }
{}

void CreateFluidSubdomain::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    auto node_name = args.arguments.at<VariableAndHash<std::string>>(KnownArgs::node);
    auto node = scene.get_node(node_name, DP_LOC);
    auto skidmark = std::make_shared<Skidmark>(Skidmark{
        .texture = nullptr,
        .vp = fixed_nans<ScenePos, 4, 4>()
        });
    auto o = object_pool.create_unique<FluidSubdomainLogic>(
        CURRENT_SOURCE_LOCATION,
        node,
        skidmark,
        args.arguments.at<UFixedArray<SceneDir, 2>>(KnownArgs::directional_velocity),
        args.arguments.at<float>(KnownArgs::radial_velocity),
        args.arguments.at<float>(KnownArgs::angular_velocity) * degrees,
        args.arguments.at<DefaultUnitialized<AxisAlignedBoundingBox<float, 2>>>(KnownArgs::velocity_region),
        args.arguments.at<int>(KnownArgs::texture_width),
        args.arguments.at<int>(KnownArgs::texture_height),
        args.arguments.at_non_null<std::chrono::steady_clock::duration>(
            KnownArgs::velocity_dt, std::chrono::steady_clock::duration{0}),
        args.arguments.at<float>(KnownArgs::speed_of_sound, 1.1f),
        args.arguments.at<float>(KnownArgs::time_relaxation_constant, 0.55f),
        args.arguments.at<float>(KnownArgs::density_normalization, 0.99f),
        args.arguments.at<float>(KnownArgs::reference_inner_velocity, 50.f) * kph,
        args.arguments.at<float>(KnownArgs::maximum_inner_velocity, 0.2f),
        args.arguments.at<Interval<float>>(KnownArgs::visual_density, Interval<float>{0.7f, 1.3f}));
    o->on_skidmark_node_clear.add([&p=object_pool, &o=*o](){ p.remove(o); }, CURRENT_SOURCE_LOCATION);
    render_logics.prepend(
        { *o, CURRENT_SOURCE_LOCATION },
        0 /* z_order */,
        CURRENT_SOURCE_LOCATION);
    node->add_skidmark(skidmark);
    o.release();
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "create_fluid_subdomain",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
                CreateFluidSubdomain(args.renderable_scene()).execute(args);
            });
    }
} obj;

}
