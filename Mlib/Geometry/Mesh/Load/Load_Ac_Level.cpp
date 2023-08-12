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
    std::list<ReplacementParameterAndFilename> result;
    auto add_level = [this, &result](
        const fs::path& stage_filename,
        const fs::path& preview_filename,
        const fs::path& ui_track_filename,
        const std::string& level_id)
    {
        auto f = create_ifstream(ui_track_filename);
        if (f->fail()) {
            THROW_OR_ABORT("Could not open file \"" + ui_track_filename.string() + '"');
        }
        nlohmann::json j;
        *f >> j;
        if (f->fail()) {
            THROW_OR_ABORT("Could not read from file \"" + ui_track_filename.string() + '"');
        }
        // Storing fields in temporary variables to
        // work around a bug in MSVC.
        auto jv = JsonView{j};
        auto globals = JsonMacroArguments({
            {"LEVEL_ICON_FILE", preview_filename},
            {"IF_RACEWAY_CIRCULAR", false},
            {"STAGE_INI_FILENAME", stage_filename}
        });
        auto required = std::vector<std::string>({"%GAME_MODE == 'rally'"});
        result.push_back(ReplacementParameterAndFilename{
            .rp = ReplacementParameter{
                .id = level_id,
                .title = jv.at<std::string>("name"),
                .globals = std::move(globals),
                .required = std::move(required)
            },
            .filename = script_filename_});
    };
    for (const auto& level_dir : list_dir(path)) {
        auto ui_dir = level_dir / fs::path{"ui"};
        if (!path_exists(ui_dir)) {
            continue;
        }
        if (auto ui_track_filename = ui_dir / fs::path{"ui_track.json"}; path_exists(ui_track_filename))
        {
            std::list<fs::path> kn5_candidates;
            for (const auto& kn5_file : list_dir(level_dir)) {
                if (kn5_file.path().extension() == ".kn5") {
                    kn5_candidates.push_back(kn5_file);
                }
            }
            if (kn5_candidates.size() != 1) {
                THROW_OR_ABORT("Did not find exactly on .kn5-file in \"" + level_dir.path().string() + '"');
            }
            add_level(
                kn5_candidates.front(),
                ui_dir / fs::path{"preview.png"},
                ui_track_filename,
                kn5_candidates.front().stem().string());
        } else {
            for (const auto& stage_dir : list_dir(ui_dir)) {
                if (!is_listable(stage_dir)) {
                    continue;
                }
                auto level_id = stage_dir.path().filename().string();
                add_level(
                    (level_dir / fs::path{"models_" + level_id}).string() + ".ini",
                    stage_dir.path() / fs::path{"preview.png"},
                    stage_dir.path() / fs::path{"ui_track.json"},
                    level_id);
            }
        }
    }
    return result;
}
