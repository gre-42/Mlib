#include "Create_Copy_Rotation.hpp"
#include <Mlib/Physics/Advance_Times/Movables/Copy_Rotation.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
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

const std::string CreateCopyRotation::key = "copy_rotation";

LoadSceneUserFunction CreateCopyRotation::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^from=\\s*([\\w+-.]+)"
        "\\s+to=\\s*([\\w+-.]+)$");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(args.line, match, regex)) {
        THROW_OR_ABORT("Could not parse user function arguments");
    }
    CreateCopyRotation(args.renderable_scene()).execute(match, args);
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
    auto rt = std::make_unique<CopyRotation>(
        physics_engine.advance_times_, from);
    auto& rt_p = *rt;
    linker.link_relative_movable(to, std::move(rt));
    from.destruction_observers.add(rt_p);
}
