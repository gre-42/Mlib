#pragma once
#include <Mlib/Macro_Recorder.hpp>
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
        std::map<std::string, std::shared_ptr<RenderableScene>>& renderable_scenes);
private:
    MacroRecorder macro_file_executor_;
};

}
