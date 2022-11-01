#include "Create_Copy_Rotation.hpp"
#include <Mlib/Physics/Advance_Times/Movables/Copy_Rotation.hpp>
#include <Mlib/Physics/Physics_Engine.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/Linker.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(FROM);
DECLARE_OPTION(TO);

LoadSceneUserFunction CreateCopyRotation::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*copy_rotation"
        "\\s+from=\\s*([\\w+-.]+)"
        "\\s+to=\\s*([\\w+-.]+)$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        CreateCopyRotation(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

CreateCopyRotation::CreateCopyRotation(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateCopyRotation::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    Linker linker{ physics_engine.advance_times_ };
    auto& from = scene.get_node(match[FROM].str());
    auto& to = scene.get_node(match[TO].str());
    auto rt = std::make_shared<CopyRotation>(
        physics_engine.advance_times_, from);
    linker.link_relative_movable(to, rt);
    from.destruction_observers.add(rt.get());
}
