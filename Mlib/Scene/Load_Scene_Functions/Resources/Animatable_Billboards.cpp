#include <Mlib/Geometry/Material/Particle_Type.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/OpenGL/Batch_Renderers/Particles_Instance.hpp>
#include <Mlib/OpenGL/Rendering_Context.hpp>
#include <Mlib/OpenGL/Resource_Managers/Particle_Resources.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <stdexcept>
#include <vector>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(billboards);
DECLARE_ARGUMENT(max_num_instances);
DECLARE_ARGUMENT(type);
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "animatable_billboards",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);

                RenderingContextStack::primary_particle_resources().insert_instance_instantiator(
                    args.arguments.at<VariableAndHash<std::string>>(KnownArgs::name),
                    [&snr = RenderingContextStack::primary_scene_node_resources(),
                     billboards = args.arguments.at<VariableAndHash<std::string>>(KnownArgs::billboards),
                     max_num_instances = args.arguments.at<size_t>(KnownArgs::max_num_instances),
                     filter = RenderableResourceFilter{},
                     type = particle_type_from_string(args.arguments.at<std::string>(KnownArgs::type))]
                    ()
                    {
                        auto scva = snr.get_single_precision_array(billboards, filter.cva_filter);
                        auto gvd = snr.get_gpu_vertex_data(scva, nullptr);
                        return std::make_shared<ParticlesInstance>(gvd, max_num_instances, filter, type);
                    });
            });
    }
} obj;

}
