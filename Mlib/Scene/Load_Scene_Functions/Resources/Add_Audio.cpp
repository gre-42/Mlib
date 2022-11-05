#include "Add_Audio.hpp"
#include <Mlib/Audio/Audio_Resource_Context.hpp>
#include <Mlib/Audio/Audio_Resources.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Strings/To_Number.hpp>

using namespace Mlib;

LoadSceneUserFunction AddAudio::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*add_audio"
        "\\s+name=([\\w+-.]+)"
        "\\s+filename=([\\w+-. \\(\\)/]+)"
        "\\s+gain=([\\w+-.]+)$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        execute(match, args);
        return true;
    } else {
        return false;
    }
};

void AddAudio::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    AudioResourceContextStack::primary_audio_resources()->add_buffer(
        match[1].str(),
        match[2].str(),
        match[3].matched ? safe_stof(match[3].str()) : 1);
}
