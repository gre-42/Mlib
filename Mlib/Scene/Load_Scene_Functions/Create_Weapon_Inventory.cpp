#include "Create_Weapon_Inventory.hpp"
#include <Mlib/Physics/Misc/Weapon_Inventory.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(STORAGE_NODE);

LoadSceneInstanceFunction::UserFunction CreateWeaponInventory::user_function = [](
    const std::string& line,
    const std::function<RenderableScene&()>& renderable_scene,
    const std::function<FPath(const std::string&)>& fpath,
    const MacroLineExecutor& macro_line_executor,
    SubstitutionMap& external_substitutions,
    SubstitutionMap* local_substitutions,
    RegexSubstitutionCache& rsc)
{
    static DECLARE_REGEX(regex,
        "^\\s*create_weapon_inventory"
        "\\s+storage_node=([\\w+-.]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(line, match, regex)) {
        CreateWeaponInventory(renderable_scene()).execute(
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

CreateWeaponInventory::CreateWeaponInventory(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateWeaponInventory::execute(
    const std::smatch& match,
    const std::function<FPath(const std::string&)>& fpath,
    const MacroLineExecutor& macro_line_executor,
    SubstitutionMap* local_substitutions,
    RegexSubstitutionCache& rsc)
{
    auto storage_node = scene.get_node(match[STORAGE_NODE].str());
    storage_node->set_node_modifier(std::make_unique<WeaponInventory>());
}
