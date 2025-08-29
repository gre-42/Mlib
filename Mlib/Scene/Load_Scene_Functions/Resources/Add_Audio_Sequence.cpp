#include "Add_Audio_Sequence.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Audio/Audio_Resource_Context.hpp>
#include <Mlib/Audio/Audio_Resources.hpp>
#include <Mlib/Geometry/Intersection/Interval_Json.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Strings/To_Number.hpp>

using namespace Mlib;

const std::string AddAudioSequence::key = "add_audio_sequence";

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(filename);
DECLARE_ARGUMENT(gain);
DECLARE_ARGUMENT(distance_clamping);
DECLARE_ARGUMENT(hysteresis_step);
}

LoadSceneJsonUserFunction AddAudioSequence::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    AudioResourceContextStack::primary_audio_resources()->add_buffer_sequence(
        args.arguments.at<VariableAndHash<std::string>>(KnownArgs::name),
        args.arguments.path(KnownArgs::filename),
        args.arguments.at<float>(KnownArgs::gain, 1.f),
        args.arguments.try_at<Interval<float>>(KnownArgs::distance_clamping),
        args.arguments.at<float>(KnownArgs::hysteresis_step));
};
