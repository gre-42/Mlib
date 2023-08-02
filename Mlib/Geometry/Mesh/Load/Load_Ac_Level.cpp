#include "Load_Ac_Level.hpp"
#include <Mlib/Json/Misc.hpp>
#include <Mlib/Macro_Executor/Replacement_Parameter.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <filesystem>

namespace fs = std::filesystem;
using namespace Mlib;

LoadAcLevel::LoadAcLevel(std::string script_filename)
: script_filename_{std::move(script_filename)}
{}

LoadAcLevel::~LoadAcLevel() = default;

std::list<ReplacementParameterAndFilename> LoadAcLevel::try_load(const std::string& path) {
    auto ui_dir = path / fs::path{"ui"};
    if (!path_exists(ui_dir)) {
        return {};
    }
    std::list<ReplacementParameterAndFilename> result;
    for (auto const& stage_dir : list_dir(ui_dir)) {
        if (!is_listable(stage_dir)) {
            continue;
        }
        auto filename = stage_dir.path() / fs::path{"ui_track.json"};
        auto f = create_ifstream(filename);
        if (f->fail()) {
            THROW_OR_ABORT("Could not open file \"" + filename.string() + '"');
        }
        nlohmann::json j;
        *f >> j;
        if (f->fail()) {
            THROW_OR_ABORT("Could not read from file \"" + filename.string() + '"');
        }
        auto level_id = stage_dir.path().filename().string();
        result.push_back(ReplacementParameterAndFilename{
            .rp = ReplacementParameter{
                .id = level_id,
                .title = JsonView{j}.at<std::string>("name"),
                .globals = JsonMacroArguments({
                    {"LEVEL_ICON_FILE", "#black"},
                    {"IF_RACEWAY_CIRCULAR", false},
                    {"STAGE_INI_FILENAME", (path / fs::path{"models_" + level_id}).string() + ".ini"}
                })
            },
            .filename = script_filename_});
    }
    return result;
}
