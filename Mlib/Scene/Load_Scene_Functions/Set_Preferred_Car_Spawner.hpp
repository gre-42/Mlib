#include <Mlib/Scene/Load_Scene_Instance_Function.hpp>
#include <regex>

namespace Mlib {

class SetPreferredCarSpawner: public LoadSceneInstanceFunction {
public:
    static UserFunction user_function;
private:
    explicit SetPreferredCarSpawner(RenderableScene& renderable_scene);
    void execute(
        const std::smatch& match,
        const std::function<FPath(const std::string&)>& fpath,
        const MacroLineExecutor& macro_line_executor,
        SubstitutionMap* local_substitutions,
        RegexSubstitutionCache& rsc);
};

}
