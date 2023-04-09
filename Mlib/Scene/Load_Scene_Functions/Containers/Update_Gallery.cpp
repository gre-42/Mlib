#include "Update_Gallery.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Render_Logic_Gallery.hpp>
#include <Mlib/Render/Render_Logics/Fill_With_Texture_Logic.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(RESOURCE);
DECLARE_OPTION(INSTANCE);

const std::string UpdateGallery::key = "update_gallery";

LoadSceneUserFunction UpdateGallery::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^resource=(#?[\\w+-.\\(\\)/]+)"
        "\\s+instance=([\\w+-.]+)$");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(args.line, match, regex)) {
        THROW_OR_ABORT("Could not parse user function arguments");
    }
    execute(match, args);
};

void UpdateGallery::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto entry = args.gallery[match[INSTANCE].str()];
    entry->set_image_resource_name(args.fpath(match[RESOURCE].str()).path);
}
