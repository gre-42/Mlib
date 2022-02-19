#include "Extract_Alignment_Planes.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Strings/From_Number.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(SOURCE_NAME);
DECLARE_OPTION(DEST_NAME);
DECLARE_OPTION(OBJECT_PREFIX);

LoadSceneUserFunction ExtractAlignmentPlanes::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*extract_alignment_planes"
        "\\s+source_name=([\\w+-.]+)"
        "\\s+dest_name=([\\w+-.]+)"
        "\\s+object_prefix=([\\w+-.]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        execute(match, args);
        return true;
    } else {
        return false;
    }
};

void ExtractAlignmentPlanes::execute(
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    args.scene_node_resources.extract_alignment_planes(
        match[SOURCE_NAME].str(),
        match[DEST_NAME].str(),
        match[OBJECT_PREFIX].str());
}
