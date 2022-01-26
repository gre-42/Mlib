#include "Create_Avatar_Controller_Idle_Binding.hpp"
#include <Mlib/Macro_Line_Executor.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Key_Bindings/Avatar_Controller_Idle_Binding.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NODE);

LoadSceneInstanceFunction::UserFunction CreateAvatarControllerIdleBinding::user_function = [](
    const std::string& line,
    const std::function<RenderableScene&()>& renderable_scene,
    const std::function<FPath(const std::string&)>& fpath,
    const MacroLineExecutor& macro_line_executor,
    SubstitutionMap& external_substitutions,
    SubstitutionMap* local_substitutions,
    RegexSubstitutionCache& rsc)
{
    static DECLARE_REGEX(regex,
        "^\\s*avatar_controller_idle_binding"
        "\\s+node=([\\w+-.]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(line, match, regex)) {
        CreateAvatarControllerIdleBinding(renderable_scene()).execute(
            match,
            fpath,
            macro_line_executor,
            local_substitutions,
            rsc);
        return true;
    } else {
        return false;
    }
};

CreateAvatarControllerIdleBinding::CreateAvatarControllerIdleBinding(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateAvatarControllerIdleBinding::execute(
    const std::smatch& match,
    const std::function<FPath(const std::string&)>& fpath,
    const MacroLineExecutor& macro_line_executor,
    SubstitutionMap* local_substitutions,
    RegexSubstitutionCache& rsc)
{
    key_bindings.add_avatar_controller_idle_binding(AvatarControllerIdleBinding{
        .node = scene.get_node(match[NODE].str())});
}
