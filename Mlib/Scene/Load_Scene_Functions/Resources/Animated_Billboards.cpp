#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/OpenGL/Batch_Renderers/Particle_Creator.hpp>
#include <Mlib/OpenGL/Batch_Renderers/Particles_Instance.hpp>
#include <Mlib/OpenGL/Rendering_Context.hpp>
#include <Mlib/OpenGL/Resource_Managers/Particle_Resources.hpp>
#include <Mlib/OpenGL/Resources/Colored_Vertex_Array_Resource/Billboard_Sequence.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <stdexcept>
#include <vector>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(animatable);
DECLARE_ARGUMENT(frames);
DECLARE_ARGUMENT(duration);
DECLARE_ARGUMENT(final_texture_w);
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "animated_billboards",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
    
                auto name = args.arguments.at<VariableAndHash<std::string>>(KnownArgs::name);
                auto animatable = VariableAndHash{ args.arguments.at<std::string>(KnownArgs::animatable) };

                auto& pr = RenderingContextStack::primary_particle_resources();
                pr.insert_creator_instantiator(
                    name,
                    [frames = args.arguments.at<std::vector<BillboardId>>(KnownArgs::frames),
                     duration = args.arguments.at<float>(KnownArgs::duration),
                     final_texture_w = args.arguments.at<float>(KnownArgs::final_texture_w, 0.f)]
                    (ParticlesInstance& particles_instance)
                    {
                        return std::unique_ptr<IParticleCreator>(new ParticleCreator(
                            particles_instance,
                            BillboardSequence{
                                .billboard_ids = frames,
                                .duration = duration * seconds,
                                .final_texture_w = final_texture_w}));
                    });
                pr.insert_creator_to_instance(std::move(name), std::move(animatable));
            });
    }
} obj;

}
