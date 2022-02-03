#pragma once
#include <functional>
#include <string>

namespace Mlib {

class MacroLineExecutor;
class SubstitutionMap;
class RegexSubstitutionCache;
struct UiFocus;
class RenderableScene;
class FPath;
class SceneNodeResources;

struct LoadSceneUserFunctionArgs {
    const std::string& line;
    const std::function<RenderableScene&()>& renderable_scene;
    const std::function<FPath(const std::string&)>& fpath;
    const MacroLineExecutor& macro_line_executor;
    SubstitutionMap& external_substitutions;
    SubstitutionMap* local_substitutions;
    RegexSubstitutionCache& rsc;
    UiFocus& ui_focus;
    SceneNodeResources& scene_node_resources;
    size_t& num_renderings;
    const std::string& script_filename;
    std::string& next_scene_filename;
};

}
