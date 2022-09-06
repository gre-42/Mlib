#include "Set_Node_Bone_Name.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NODE);
DECLARE_OPTION(BONE);

LoadSceneUserFunction SetNodeBoneName::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*set_node_bone_name"
        "\\s*node=([\\w+-.]+)"
        "\\s*bone=([\\w+-.]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        SetNodeBoneName(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

SetNodeBoneName::SetNodeBoneName(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetNodeBoneName::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    scene.get_node(match[NODE].str())
        .set_bone_name(match[BONE].str());
}
