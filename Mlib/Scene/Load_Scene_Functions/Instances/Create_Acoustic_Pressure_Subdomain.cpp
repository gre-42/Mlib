#include "Create_Acoustic_Pressure_Subdomain.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box_Json.hpp>
#include <Mlib/Geometry/Material/Particle_Type.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Acoustic_Pressure_Subdomain_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
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
DECLARE_ARGUMENT(inner_pressure);
DECLARE_ARGUMENT(inner_region);
DECLARE_ARGUMENT(inner_angular_velocity);
DECLARE_ARGUMENT(wind_amplitude);
DECLARE_ARGUMENT(wind_angular_velocity);
DECLARE_ARGUMENT(wind_cutoff);
DECLARE_ARGUMENT(wind_texture);
DECLARE_ARGUMENT(c);
DECLARE_ARGUMENT(dt);
DECLARE_ARGUMENT(dx);
DECLARE_ARGUMENT(intensity_normalization);
DECLARE_ARGUMENT(pressure_limitation);
DECLARE_ARGUMENT(periodicity);
DECLARE_ARGUMENT(skidmark_strength);
}

CreateAcousticPressureSubdomain::CreateAcousticPressureSubdomain(RenderableScene& renderable_scene) 
    : LoadRenderableSceneInstanceFunction{ renderable_scene }
{}

void CreateAcousticPressureSubdomain::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    auto node_name = args.arguments.at<VariableAndHash<std::string>>(KnownArgs::node);
    auto node = scene.get_node(node_name, DP_LOC);
    auto skidmark = std::make_shared<Skidmark>(Skidmark{
        .particle_type = ParticleType::WATER_WAVE,
        .texture = nullptr,
        .vp = fixed_nans<ScenePos, 4, 4>()
        });
    auto o = object_pool.create_unique<AcousticPressureSubdomainLogic>(
        CURRENT_SOURCE_LOCATION,
        node,
        skidmark,
        args.arguments.at<float>(KnownArgs::inner_pressure),
        args.arguments.at<float>(KnownArgs::inner_angular_velocity) * degrees,
        args.arguments.at<DefaultUnitialized<AxisAlignedBoundingBox<float, 2>>>(KnownArgs::inner_region),
        args.arguments.at<float>(KnownArgs::wind_amplitude),
        args.arguments.at<float>(KnownArgs::wind_angular_velocity) * degrees,
        args.arguments.at<float>(KnownArgs::wind_cutoff),
        RenderingContextStack::primary_rendering_resources().get_texture_lazy(
            ColormapWithModifiers{
                .filename = VariableAndHash{ args.arguments.path_or_variable(KnownArgs::wind_texture).path },
                .color_mode = ColorMode::GRAYSCALE
            }.compute_hash(),
            TextureRole::COLOR_FROM_DB),
        args.arguments.at<int>(KnownArgs::texture_width),
        args.arguments.at<int>(KnownArgs::texture_height),
        args.arguments.at<float>(KnownArgs::c),
        args.arguments.at<float>(KnownArgs::dt),
        args.arguments.at<float>(KnownArgs::dx),
        args.arguments.at<float>(KnownArgs::intensity_normalization),
        args.arguments.at<BoundaryLimitation>(KnownArgs::pressure_limitation),
        periodicity_from_string(args.arguments.at<std::string>(KnownArgs::periodicity)),
        args.arguments.at<float>(KnownArgs::skidmark_strength));
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
            "create_acoustic_pressure_subdomain",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
                CreateAcousticPressureSubdomain(args.renderable_scene()).execute(args);
            });
    }
} obj;

}
