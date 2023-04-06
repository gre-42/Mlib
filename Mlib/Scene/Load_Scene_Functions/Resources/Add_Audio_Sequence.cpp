#include "Add_Audio_Sequence.hpp"
#include <Mlib/Audio/Audio_Resource_Context.hpp>
#include <Mlib/Audio/Audio_Resources.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Strings/To_Number.hpp>

using namespace Mlib;

LoadSceneJsonUserFunction AddAudioSequence::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    if (args.name == "add_audio_sequence") {
        execute(args);
        return true;
    } else {
        return false;
    }
};

void AddAudioSequence::execute(const LoadSceneJsonUserFunctionArgs& args)
{
#ifndef WITHOUT_ALUT
    AudioResourceContextStack::primary_audio_resources()->add_buffer_sequence(
        args.arguments.get<std::string>("name"),
        args.arguments.path("filename"),
        args.arguments.get<float>("gain", 1.f));
#endif
}
