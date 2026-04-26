#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array_Filter.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/OpenGL/Batch_Renderers/Trails_Instance.hpp>
#include <Mlib/OpenGL/Rendering_Context.hpp>
#include <Mlib/OpenGL/Resource_Managers/Trail_Resources.hpp>
#include <Mlib/OpenGL/Resources/Colored_Vertex_Array_Resource/Trail_Extender.hpp>
#include <Mlib/OpenGL/Resources/Colored_Vertex_Array_Resource/Trail_Storage.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <stdexcept>
#include <vector>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(animatable);
DECLARE_ARGUMENT(model);
DECLARE_ARGUMENT(u_offset);
DECLARE_ARGUMENT(u_scale);
DECLARE_ARGUMENT(duration);
DECLARE_ARGUMENT(min_spawn_length);
DECLARE_ARGUMENT(max_spawn_length);
DECLARE_ARGUMENT(spawn_duration);
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "animated_trails",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
    
                auto name = args.arguments.at<VariableAndHash<std::string>>(KnownArgs::name);
                auto animatable = args.arguments.at<VariableAndHash<std::string>>(KnownArgs::animatable);

                auto& pt = RenderingContextStack::primary_trail_resources();
                auto& sr = RenderingContextStack::primary_scene_node_resources();
                pt.insert_storage_instantiator(
                    name,
                    [&sr,
                     model = args.arguments.at<VariableAndHash<std::string>>(KnownArgs::model),
                     u_offset = args.arguments.at<float>(KnownArgs::u_offset),
                     u_scale = args.arguments.at<float>(KnownArgs::u_scale),
                     duration = args.arguments.at<float>(KnownArgs::duration) * seconds,
                     min_spawn_length = args.arguments.at<ScenePos>(KnownArgs::min_spawn_length) * meters,
                     max_spawn_length = args.arguments.at<ScenePos>(KnownArgs::max_spawn_length) * meters,
                     spawn_duration = args.arguments.at<float>(KnownArgs::spawn_duration) * seconds]
                    (TrailsInstance& trails_instance)
                    {
                        auto m = sr.get_single_precision_array(model, ColoredVertexArrayFilter{});
                        return std::unique_ptr<ITrailStorage>(new TrailStorage(
                            trails_instance,
                            TrailSequence{
                                .u_offset = u_offset,
                                .u_scale = u_scale,
                                .times_to_w = Interp<float>{ {0.f, duration}, {0.f, duration}, OutOfRangeBehavior::EXTRAPOLATE }
                            },
                            m->triangles,
                            min_spawn_length,
                            max_spawn_length,
                            spawn_duration));
                    });
                pt.insert_storage_to_instance(name, animatable);
            });
    }
} obj;

}
