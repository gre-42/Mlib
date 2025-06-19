#include "Create_Acoustic_Subdomain.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box_Json.hpp>
#include <Mlib/Json/Chrono.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Acoustic_Subdomain_Logic.hpp>
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
DECLARE_ARGUMENT(c);
DECLARE_ARGUMENT(dt);
DECLARE_ARGUMENT(dx);
DECLARE_ARGUMENT(intensity_normalization);
DECLARE_ARGUMENT(reference_inner_directional_velocity);
DECLARE_ARGUMENT(maximum_inner_velocity);
DECLARE_ARGUMENT(velocity_limitation);
}

CreateAcousticSubdomain::CreateAcousticSubdomain(RenderableScene& renderable_scene) 
    : LoadRenderableSceneInstanceFunction{ renderable_scene }
{}

void CreateAcousticSubdomain::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    auto node_name = args.arguments.at<VariableAndHash<std::string>>(KnownArgs::node);
    auto node = scene.get_node(node_name, DP_LOC);
    auto skidmark = std::make_shared<Skidmark>(Skidmark{
        .texture = nullptr,
        .vp = fixed_nans<ScenePos, 4, 4>()
        });
    auto o = object_pool.create_unique<AcousticSubdomainLogic>(
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
        args.arguments.at<float>(KnownArgs::c),
        args.arguments.at<float>(KnownArgs::dt),
        args.arguments.at<float>(KnownArgs::dx),
        args.arguments.at<float>(KnownArgs::intensity_normalization),
        args.arguments.at<float>(KnownArgs::reference_inner_directional_velocity) * kph,
        args.arguments.at<float>(KnownArgs::maximum_inner_velocity),
        args.arguments.at<VelocityLimitation>(KnownArgs::velocity_limitation));
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
            "create_acoustic_subdomain",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
                CreateAcousticSubdomain(args.renderable_scene()).execute(args);
            });
    }
} obj;

}
