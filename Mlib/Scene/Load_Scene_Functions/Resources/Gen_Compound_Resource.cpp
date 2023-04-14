#include "Gen_Compound_Resource.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/Load_Scene_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Compound_Resource.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(SOURCE_NAMES);
DECLARE_OPTION(DEST_NAME);

const std::string GenCompoundResource::key = "compound_resource";

LoadSceneUserFunction GenCompoundResource::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^source_names=(.+?)"
        "\\s+dest_name=([\\w+-.]+)$");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(args.line, match, regex)) {
        THROW_OR_ABORT("Could not parse user function arguments");
    }
    execute(match, args);
};

void GenCompoundResource::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    args.scene_node_resources.add_resource(
        match[DEST_NAME].str(),
        std::make_shared<CompoundResource>(
            args.scene_node_resources,
            string_to_vector(match[SOURCE_NAMES].str())));
}
