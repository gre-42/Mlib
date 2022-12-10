#include "Macro_Manifest.hpp"
#include <Mlib/Os.hpp>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;

using namespace Mlib;

MacroManifest::MacroManifest(const std::string& filename) {
    if (filename.ends_with(".json")) {
        try {
            nlohmann::json j;
            auto ifs_p = create_ifstream(filename);
            auto& ifs = *ifs_p;
            if (ifs.fail()) {
                throw std::runtime_error("Could not open macro manifest file \"" + filename + '"');
            }
            ifs >> j;
            if (!ifs.eof() && ifs.fail()) {
                throw std::runtime_error("Error reading from file: \"" + filename + '"');
            }
            for (const auto& [key, value] : j["variables"].get<std::map<std::string, std::string>>()) {
                variables.insert(key, value);
            }
            if (j.contains("requires")) {
                requires_ = j["requires"].get<std::string>();
            }
            script_file = (fs::path{filename}.parent_path() / j["script_file"].get<std::string>()).string();
        } catch (const nlohmann::json::exception& e) {
            throw std::runtime_error("Error loading file \"" + filename + "\": " + e.what());
        }
    } else if (filename.ends_with(".scn")) {
        script_file = filename;
    } else {
        throw std::runtime_error("Unknown script file extension: \"" + filename + '"');
    }
}
