#include "Load_Ac_Level.hpp"
#include <Mlib/Geometry/Interfaces/IDds_Resources.hpp>
#include <Mlib/Geometry/Material/Texture_Descriptor.hpp>
#include <Mlib/Io/Ini_Parser.hpp>
#include <Mlib/Json/Misc.hpp>
#include <Mlib/Macro_Executor/Replacement_Parameter.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <filesystem>

namespace fs = std::filesystem;
using namespace Mlib;

LoadAcLevel::LoadAcLevel(std::string script_filename)
    : script_filename_{ std::move(script_filename) }
{}

LoadAcLevel::~LoadAcLevel() = default;

std::list<ReplacementParameterAndFilename> LoadAcLevel::try_load(const std::string& path) {
    std::list<ReplacementParameterAndFilename> result;
    auto add_level = [this, &result](
        const fs::path& stage_filename,
        const fs::path& preview_filename,
        const fs::path& ui_track_filename,
        const fs::path& minimap_filename,
        const fs::path& minimap_ini_filename,
        const std::string& level_id)
    {
        nlohmann::json j;
        {
            auto f = create_ifstream(ui_track_filename);
            if (f->fail()) {
                THROW_OR_ABORT("Could not open file \"" + ui_track_filename.string() + '"');
            }
            *f >> j;
            if (f->fail()) {
                THROW_OR_ABORT("Could not read from file \"" + ui_track_filename.string() + '"');
            }
        }
        IniParser ini_parser{ minimap_ini_filename.string() };
        // Storing fields in temporary variables to
        // work around a bug in MSVC.
        auto jv = JsonView{ j };
        // int minimap_x;
        // int minimap_y;
        // int minimap_comp;
        // if (!fs::exists(minimap_filename)) {
        //     minimap_comp = 0;
        // } else if (stbi_info(minimap_filename.string().c_str(), &minimap_x, &minimap_y, &minimap_comp) == 0) {
        //     THROW_OR_ABORT("Could not read size information from file \"" + minimap_filename.string() + '"');
        // }
        FixedArray<float, 2> minimap_size{
            ini_parser.get<float>("PARAMETERS", "WIDTH"),
            ini_parser.get<float>("PARAMETERS", "HEIGHT") };
        FixedArray<float, 2> minimap_offset{
            ini_parser.get<float>("PARAMETERS", "X_OFFSET"),
            ini_parser.get<float>("PARAMETERS", "Z_OFFSET") };
        bool circular;
        auto tags = jv.at<std::set<std::string>>("tags");
        auto run = jv.at<std::string>("run");
        std::transform(run.begin(), run.end(), run.begin(),
            ::tolower);
        if ((run == "point to point") ||
            (run == "point-to-point") ||
            (run == "drift!") ||
            (run == "drift") ||
            (run == "downhill") ||
            (run == "uphill") ||
            (run == "downhill") ||
            tags.contains("Drift track") ||
            tags.contains("hill climb"))
        {
            circular = false;
        } else if (
            (run == "clockwise") ||
            tags.contains("circuit"))
        {
            circular = true;
        } else {
            THROW_OR_ABORT("Unknown \"run\" parameter in file \"" + preview_filename.string() + "\": " + run);
        }
        auto globals = nlohmann::json{
            {"selected_level_id", level_id},
            {"level_icon_file", preview_filename.string()},
            {"stage_ini_filename", stage_filename.string()},
            {"minimap_file", any(minimap_size != 20.f) || any(minimap_offset != 20.f)
                ? nlohmann::json(minimap_filename.string())
                : nlohmann::json()},
            {"minimap_scale", ini_parser.get<float>("PARAMETERS", "SCALE_FACTOR")},
            {"minimap_size", minimap_size},
            {"minimap_offset", minimap_offset} };
        auto on_before_select = nlohmann::json{
            {"call", "globals"},
            {"arguments", std::move(globals)} };
        auto database = JsonMacroArguments(nlohmann::json{
            {"if_raceway_circular", circular},
            {"game_modes", std::vector<std::string>{"rally"}} });
        auto required = std::vector<std::vector<std::string>>({{ "%selected_game_mode == 'rally'" }});
        result.push_back(ReplacementParameterAndFilename{
            .rp = ReplacementParameter{
                .id = level_id,
                .title = jv.at<std::string>("name"),
                .required = {
                    .fixed = {},
                    .dynamic = std::move(required)
                },
                .database = std::move(database),
                .on_before_select = std::move(on_before_select)
            },
            .filename = script_filename_ });
    };
    for (const auto& level_dir : list_dir(path)) {
        auto ui_dir = level_dir / fs::path{ "ui" };
        if (!path_exists(ui_dir)) {
            continue;
        }
        // Single stage vs. multi stage
        if (auto ui_track_filename = ui_dir / fs::path{ "ui_track.json" }; path_exists(ui_track_filename))
        {
            // .ini file vs. single .kn5 file
            if (auto models_filename = level_dir / fs::path{ "models.ini" }; path_exists(models_filename)) {
                auto level_id = level_dir.path().filename();
                add_level(
                    models_filename,
                    ui_dir / fs::path{ "preview.png" },
                    ui_track_filename,
                    level_dir / fs::path{ "map.png" },
                    level_dir / fs::path{ "data" } / fs::path{ "map.ini" },
                    level_id.string());
            } else {
                std::list<fs::path> kn5_candidates;
                for (const auto& kn5_file : list_dir(level_dir)) {
                    if (kn5_file.path().extension() == ".kn5") {
                        kn5_candidates.push_back(kn5_file);
                    }
                }
                if (kn5_candidates.size() != 1) {
                    THROW_OR_ABORT("Did not find exactly one .kn5-file in \"" + level_dir.path().string() + '"');
                }
                add_level(
                    kn5_candidates.front(),
                    ui_dir / fs::path{ "preview.png" },
                    ui_track_filename,
                    level_dir / fs::path{ "map.png" },
                    level_dir / fs::path{ "data" } / fs::path{ "map.ini" },
                    kn5_candidates.front().stem().string());
            }
        } else {
            for (const auto& stage_dir : list_dir(ui_dir)) {
                if (!is_listable(stage_dir)) {
                    continue;
                }
                auto stage = stage_dir.path().filename();
                if (auto ui_track_filename = stage_dir / fs::path{ "ui_track.json" }; path_exists(ui_track_filename)) {
                    auto level_id = level_dir.path().filename().string() + '_' + stage.string();
                    add_level(
                        (level_dir / fs::path{ "models_" + stage.string() }).string() + ".ini",
                        stage_dir / fs::path{ "preview.png" },
                        ui_track_filename,
                        level_dir / stage / fs::path{ "map.png" },
                        level_dir / stage / fs::path{ "data" } / fs::path{ "map.ini" },
                        level_id);
                }
            }
        }
    }
    return result;
}
