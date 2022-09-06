#include "Set_Node_Bone.hpp"
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
DECLARE_OPTION(SLERP_T);

LoadSceneUserFunction SetNodeBone::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*set_node_bone"
        "\\s+node=([\\w+-.]+)"
        "\\s+bone=([\\w+-.]+)"
        "\\s+slerp_t=([\\w+-.]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        SetNodeBone(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

SetNodeBone::SetNodeBone(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetNodeBone::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    scene.get_node(match[NODE].str())
        .set_bone(SceneNodeBone{
            .name = match[BONE].str(),
            .slerp_t = safe_stof(match[SLERP_T].str())});
}
