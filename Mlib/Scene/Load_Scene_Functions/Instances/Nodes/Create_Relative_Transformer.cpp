#include "Create_Relative_Transformer.hpp"
#include <Mlib/Physics/Advance_Times/Movables/Relative_Transformer.hpp>
#include <Mlib/Physics/Physics_Engine.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/Linker.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Strings/From_Number.hpp>

using namespace Mlib;

LoadSceneUserFunction CreateRelativeTransformer::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*relative_transformer"
        "\\s+node=\\s*([\\w+-.]+)"
        "(?:\\s+v=\\s*([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+))?"
        "(?:\\s+w=\\s*([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+))?$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        CreateRelativeTransformer(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

CreateRelativeTransformer::CreateRelativeTransformer(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateRelativeTransformer::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    Linker linker{ physics_engine.advance_times_ };
    FixedArray<float, 3> v{
        match[2].str().empty() ? 0.f : safe_stof(match[2].str()) * meters / s,
        match[3].str().empty() ? 0.f : safe_stof(match[3].str()) * meters / s,
        match[4].str().empty() ? 0.f : safe_stof(match[4].str()) * meters / s};
    FixedArray<float, 3> w{
        match[5].str().empty() ? 0.f : safe_stof(match[5].str()) * degrees / s,
        match[6].str().empty() ? 0.f : safe_stof(match[6].str()) * degrees / s,
        match[7].str().empty() ? 0.f : safe_stof(match[7].str()) * degrees / s};
    auto rt = std::make_shared<RelativeTransformer>(
        physics_engine.advance_times_, v, w);
    linker.link_relative_movable(scene.get_node(match[1].str()), rt);
}
