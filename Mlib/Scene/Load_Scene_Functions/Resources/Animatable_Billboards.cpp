#include "Animatable_Billboards.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Batch_Renderers/Particles_Instance.hpp>
#include <Mlib/Render/Particle_Resources.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <vector>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(billboards);
DECLARE_ARGUMENT(max_num_instances);
}

const std::string AnimatableBillboards::key = "animatable_billboards";

LoadSceneJsonUserFunction AnimatableBillboards::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);

    auto billboards = args.arguments.at<std::string>(KnownArgs::billboards);
    auto max_num_instances = args.arguments.at<size_t>(KnownArgs::max_num_instances);

    RenderingContextStack::primary_particle_resources().insert_instance_creator(
        args.arguments.at<std::string>(KnownArgs::name),
        [&snr=RenderingContextStack::primary_scene_node_resources(),
         billboards,
         max_num_instances](){
            auto scva = snr.get_single_precision_array(billboards);
            return std::make_shared<ParticlesInstance>(scva, max_num_instances);
        });
};
