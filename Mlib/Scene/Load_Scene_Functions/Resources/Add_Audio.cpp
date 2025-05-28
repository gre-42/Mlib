#include "Add_Audio.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Audio/Audio_Resource_Context.hpp>
#include <Mlib/Audio/Audio_Resources.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Strings/To_Number.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(filename);
DECLARE_ARGUMENT(gain);
}

const std::string AddAudio::key = "add_audio";

LoadSceneJsonUserFunction AddAudio::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    execute(args);
};

void AddAudio::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    AudioResourceContextStack::primary_audio_resources()->add_buffer(
        args.arguments.at<std::string>(KnownArgs::name),
        args.arguments.path(KnownArgs::filename),
        args.arguments.at<float>(KnownArgs::gain, 1.f));
}
