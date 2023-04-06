#include "Macro_Manifest.hpp"
#include <Mlib/Json.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

using namespace Mlib;

MacroManifest MacroManifest::from_json(const std::string& filename) {
    MacroManifest result;
    if (filename.ends_with(".scn") || filename.ends_with(".scn.json")) {
        result.script_file = filename;
    } else if (filename.ends_with(".json")) {
        try {
            nlohmann::json j;
            auto ifs_p = create_ifstream(filename);
            auto& ifs = *ifs_p;
            if (ifs.fail()) {
                THROW_OR_ABORT("Could not open macro manifest file \"" + filename + '"');
            }
            ifs >> j;
            if (!ifs.eof() && ifs.fail()) {
                THROW_OR_ABORT("Error reading from file: \"" + filename + '"');
            }
            for (const auto& [key, value] : j.at("variables").get<std::map<std::string, std::string>>()) {
                result.text_variables.insert(key, value);
            }
            if (j.contains("requires")) {
                result.text_requires_ = j["requires"].get<std::vector<std::string>>();
            }
            result.script_file = (fs::path{filename}.parent_path() / j.at("script_file").get<std::string>()).string();
            result.name = j.at("name");
        } catch (const nlohmann::json::exception& e) {
            throw std::runtime_error("Error loading file \"" + filename + "\": " + e.what());
        }
    } else {
        THROW_OR_ABORT("Unknown script file extension: \"" + filename + '"');
    }
    return result;
}
