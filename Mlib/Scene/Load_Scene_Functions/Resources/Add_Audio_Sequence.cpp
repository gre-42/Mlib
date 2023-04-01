#include "Add_Audio_Sequence.hpp"
#include <Mlib/Audio/Audio_Resource_Context.hpp>
#include <Mlib/Audio/Audio_Resources.hpp>
#include <Mlib/FPath.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Strings/To_Number.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NAME);
DECLARE_OPTION(FILENAME);
DECLARE_OPTION(GAIN);

LoadSceneUserFunction AddAudioSequence::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*add_audio_sequence"
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

void AddAudioSequence::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
#ifndef WITHOUT_ALUT
    AudioResourceContextStack::primary_audio_resources()->add_buffer_sequence(
        match[NAME].str(),
        args.fpath(match[FILENAME].str()).path,
        match[GAIN].matched ? safe_stof(match[GAIN].str()) : 1.f);
#endif
}
