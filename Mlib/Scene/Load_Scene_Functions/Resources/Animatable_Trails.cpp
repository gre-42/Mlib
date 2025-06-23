#include "Animatable_Trails.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Material/Shading.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Render/Batch_Renderers/Trails_Instance.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resource_Managers/Trail_Resources.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <vector>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(texture);
DECLARE_ARGUMENT(emissive);
DECLARE_ARGUMENT(ambient);
DECLARE_ARGUMENT(diffuse);
DECLARE_ARGUMENT(specular);
DECLARE_ARGUMENT(times);
DECLARE_ARGUMENT(w);
DECLARE_ARGUMENT(max_num_triangles);
}

const std::string AnimatableTrails::key = "animatable_trails";

static float as_seconds(float v) {
    return v * seconds;
}

LoadSceneJsonUserFunction AnimatableTrails::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);

    auto zeros3 = fixed_zeros<float, 3>();
    RenderingContextStack::primary_trail_resources().insert_instance_instantiator(
        args.arguments.at<VariableAndHash<std::string>>(KnownArgs::name),
        [&snr = RenderingContextStack::primary_scene_node_resources(),
         texture = args.arguments.at<VariableAndHash<std::string>>(KnownArgs::texture),
         shading = Shading{
            .emissive = OrderableFixedArray(args.arguments.at<UFixedArray<float, 3>>(KnownArgs::emissive, zeros3)),
            .ambient = OrderableFixedArray(args.arguments.at<UFixedArray<float, 3>>(KnownArgs::ambient, zeros3)),
            .diffuse = OrderableFixedArray(args.arguments.at<UFixedArray<float, 3>>(KnownArgs::diffuse, zeros3)),
            .specular = OrderableFixedArray(args.arguments.at<UFixedArray<float, 3>>(KnownArgs::specular, zeros3))
         },
         times = args.arguments.at_vector<float>(KnownArgs::times, as_seconds),
         w = args.arguments.at<std::vector<float>>(KnownArgs::w),
         max_num_triangles = args.arguments.at<size_t>(KnownArgs::max_num_triangles),
         filter = RenderableResourceFilter{}]
        ()
        {
            return std::make_shared<TrailsInstance>(texture, shading, times, w, max_num_triangles, filter);
        });
};
