#include "Add_Cubemap.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Geometry/Material/Texture_Descriptor.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>

using namespace Mlib;

LoadSceneUserFunction AddCubemap::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*add_cubemap"
        "\\s+alias=([\\w+-.]+)"
        "\\s+filenames=\\s*"
        "\\s+([\\w-. \\(\\)/+-]+),"
        "\\s+([\\w-. \\(\\)/+-]+),"
        "\\s+([\\w-. \\(\\)/+-]+),"
        "\\s+([\\w-. \\(\\)/+-]+),"
        "\\s+([\\w-. \\(\\)/+-]+),"
        "\\s+([\\w-. \\(\\)/+-]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        execute(match, args);
        return true;
    } else {
        return false;
    }
};

void AddCubemap::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    RenderingContextStack::primary_rendering_resources()->get_cubemap(
        match[1].str(),
        {
            args.fpath(match[2].str()).path,
            args.fpath(match[3].str()).path,
            args.fpath(match[4].str()).path,
            args.fpath(match[5].str()).path,
            args.fpath(match[6].str()).path,
            args.fpath(match[7].str()).path
        });
}
