#include "Animated_Trails.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Render/Batch_Renderers/Trails_Instance.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Trail_Extender.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Trail_Storage.hpp>
#include <Mlib/Render/Trail_Resources.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <vector>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(animatable);
DECLARE_ARGUMENT(model);
DECLARE_ARGUMENT(u_offset);
DECLARE_ARGUMENT(u_scale);
DECLARE_ARGUMENT(frames);
DECLARE_ARGUMENT(times);
DECLARE_ARGUMENT(minimum_length);
}

const std::string AnimatedTrails::key = "animated_trails";

inline float to_seconds(float value) {
    return value * s;
}

LoadSceneJsonUserFunction AnimatedTrails::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    
    auto name = args.arguments.at<std::string>(KnownArgs::name);
    auto animatable = args.arguments.at<std::string>(KnownArgs::animatable);

    auto& pt = RenderingContextStack::primary_trail_resources();
    auto& sr = RenderingContextStack::primary_scene_node_resources();
    pt.insert_storage_instantiator(
        name,
        [&sr,
         model = args.arguments.at<std::string>(KnownArgs::model),
         u_offset = args.arguments.at<float>(KnownArgs::u_offset),
         u_scale = args.arguments.at<float>(KnownArgs::u_scale),
         frames = args.arguments.at<std::vector<float>>(KnownArgs::frames),
         times = args.arguments.at_vector<float>(KnownArgs::times, to_seconds),
         minimum_length = args.arguments.at<double>(KnownArgs::minimum_length)]
        (TrailsInstance& trails_instance)
        {
            auto m = sr.get_single_precision_array(model);
            return std::unique_ptr<ITrailStorage>(new TrailStorage(
                trails_instance,
                TrailSequence{
                    .u_offset = u_offset,
                    .u_scale = u_scale,
                    .times_to_frames = Interp<float>{ times, frames, OutOfRangeBehavior::CLAMP }
                },
                m->triangles,
                minimum_length));
        });
    pt.insert_storage_to_instance(name, animatable);
};
