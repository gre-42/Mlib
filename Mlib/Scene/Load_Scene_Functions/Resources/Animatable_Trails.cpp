#include "Animatable_Trails.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Material/Shading.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
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

LoadSceneJsonUserFunction AnimatableTrails::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);

    RenderingContextStack::primary_trail_resources().insert_instance_instantiator(
        args.arguments.at<std::string>(KnownArgs::name),
        [&snr = RenderingContextStack::primary_scene_node_resources(),
         texture = args.arguments.at<std::string>(KnownArgs::texture),
         shading = Shading{
            .emissive = OrderableFixedArray{ args.arguments.at<FixedArray<float, 3>>(KnownArgs::emissive, fixed_zeros<float, 3>()) },
            .ambient = OrderableFixedArray{ args.arguments.at<FixedArray<float, 3>>(KnownArgs::ambient, fixed_zeros<float, 3>()) },
            .diffuse = OrderableFixedArray{ args.arguments.at<FixedArray<float, 3>>(KnownArgs::diffuse, fixed_zeros<float, 3>()) },
            .specular = OrderableFixedArray{ args.arguments.at<FixedArray<float, 3>>(KnownArgs::specular, fixed_zeros<float, 3>()) }
         },
         times = args.arguments.at<std::vector<float>>(KnownArgs::times),
         w = args.arguments.at<std::vector<float>>(KnownArgs::w),
         max_num_triangles = args.arguments.at<size_t>(KnownArgs::max_num_triangles),
         filter = RenderableResourceFilter{}]
        ()
        {
            return std::make_shared<TrailsInstance>(texture, shading, times, w, max_num_triangles, filter);
        });
};
