#include "Set_Node_Rotation.hpp"
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NAME);

DECLARE_OPTION(ROTATION_X);
DECLARE_OPTION(ROTATION_Y);
DECLARE_OPTION(ROTATION_Z);

LoadSceneUserFunction SetNodeRotation::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*set_node_rotation"
        "\\s+name=([\\w+-.]+)"
        "\\s+rotation=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        SetNodeRotation(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

SetNodeRotation::SetNodeRotation(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetNodeRotation::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto& node = scene.get_node(match[NAME].str());
    node.set_rotation(FixedArray<float, 3>{
        safe_stof(match[ROTATION_X].str()) * degrees,
        safe_stof(match[ROTATION_Y].str()) * degrees,
        safe_stof(match[ROTATION_Z].str()) * degrees});
}