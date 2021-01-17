#pragma once
#include <Mlib/Macro_File_Executor.hpp>
#include <memory>

namespace Mlib {

class RenderableScene;
class SubstitutionString;

class LoadScene {
public:
    void operator () (
        const std::string& scene_filename,
        const std::string& script_filename,
        std::string& next_scene_filename,
        SubstitutionString& substitutions,
        size_t& num_renderings,
        bool verbose,
        RegexSubstitutionCache& rsc,
        std::map<std::string, std::unique_ptr<RenderableScene>>& renderable_scenes);
private:
    MacroFileExecutor macro_file_executor_;
};

}
