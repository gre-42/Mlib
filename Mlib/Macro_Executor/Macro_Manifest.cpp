#include "Macro_Manifest.hpp"
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;

using namespace Mlib;

MacroManifest::MacroManifest(const std::string& filename) {
    if (filename.ends_with(".json")) {
        try {
            nlohmann::json j;
            std::ifstream ifs{filename};
            if (ifs.fail()) {
                throw std::runtime_error("Could not open file \"" + filename + '"');
            }
            ifs >> j;
            if (!ifs.eof() && ifs.fail()) {
                throw std::runtime_error("Error reading from file: \"" + filename + '"');
            }
            for (const auto& [key, value] : j["variables"].get<std::map<std::string, std::string>>()) {
                variables.insert(key, value);
            }
            script_file = (fs::path{filename}.parent_path() / j["script_file"].get<std::string>()).string();
        } catch (const nlohmann::json::exception& e) {
            throw std::runtime_error("Error loading file \"" + filename + "\": " + e.what());
        }
    } else if (filename.ends_with(".scn")) {
        script_file = filename;
    }
}
