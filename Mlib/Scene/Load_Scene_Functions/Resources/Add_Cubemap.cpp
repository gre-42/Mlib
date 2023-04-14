#include "Add_Cubemap.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Geometry/Material/Texture_Descriptor.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Scene/Load_Scene_User_Function_Args.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(ALIAS);
DECLARE_OPTION(DESATURATE);
DECLARE_OPTION(FILENAMES_0);
DECLARE_OPTION(FILENAMES_1);
DECLARE_OPTION(FILENAMES_2);
DECLARE_OPTION(FILENAMES_3);
DECLARE_OPTION(FILENAMES_4);
DECLARE_OPTION(FILENAMES_5);

const std::string AddCubemap::key = "add_cubemap";

LoadSceneUserFunction AddCubemap::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^alias=([\\w+-.]+)"
        "(?:\\s+desaturate=(0|1))?"
        "\\s+filenames=\\s*"
        "\\s+([\\w+-. \\(\\)/]+),"
        "\\s+([\\w+-. \\(\\)/]+),"
        "\\s+([\\w+-. \\(\\)/]+),"
        "\\s+([\\w+-. \\(\\)/]+),"
        "\\s+([\\w+-. \\(\\)/]+),"
        "\\s+([\\w+-. \\(\\)/]+)$");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(args.line, match, regex)) {
        THROW_OR_ABORT("Could not parse user function arguments");
    }
    execute(match, args);
};

void AddCubemap::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    RenderingContextStack::primary_rendering_resources()->add_cubemap(
        match[ALIAS].str(),
        {
            args.fpath(match[FILENAMES_0].str()).path,
            args.fpath(match[FILENAMES_1].str()).path,
            args.fpath(match[FILENAMES_2].str()).path,
            args.fpath(match[FILENAMES_3].str()).path,
            args.fpath(match[FILENAMES_4].str()).path,
            args.fpath(match[FILENAMES_5].str()).path
        },
        match[DESATURATE].matched ? safe_stob(match[DESATURATE].str()) : 0);
}
