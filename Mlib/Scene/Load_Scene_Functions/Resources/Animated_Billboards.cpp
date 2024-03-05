#include "Animated_Billboards.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Render/Batch_Renderers/Particle_Creator.hpp>
#include <Mlib/Render/Batch_Renderers/Particles_Instance.hpp>
#include <Mlib/Render/Particle_Resources.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Billboard_Sequence.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <vector>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(animatable);
DECLARE_ARGUMENT(frames);
DECLARE_ARGUMENT(duration);
}

const std::string AnimatedBillboards::key = "animated_billboards";

LoadSceneJsonUserFunction AnimatedBillboards::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    
    auto name = args.arguments.at<std::string>(KnownArgs::name);
    auto animatable = args.arguments.at<std::string>(KnownArgs::animatable);

    auto& pr = RenderingContextStack::primary_particle_resources();
    pr.insert_creator_instantiator(
        name,
        [frames = args.arguments.at<std::vector<uint32_t>>(KnownArgs::frames),
         duration = args.arguments.at<float>(KnownArgs::duration)]
        (ParticlesInstance& particles_instance)
        {
            return std::unique_ptr<IParticleCreator>(new ParticleCreator(
                particles_instance,
                BillboardSequence{
                    .billboard_ids = frames,
                    .duration = duration * s}));
        });
    pr.insert_creator_to_instance(name, animatable);
};
