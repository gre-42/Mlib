#include "Replacement_Parameter.hpp"
#include <Mlib/Json.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

using namespace Mlib;

ReplacementParameter ReplacementParameter::from_json(const std::string& filename) {
    ReplacementParameter result;
    try {
        nlohmann::json j;
        auto ifs_p = create_ifstream(filename);
        auto& ifs = *ifs_p;
        if (ifs.fail()) {
            THROW_OR_ABORT("Could not open replacement parameter file \"" + filename + '"');
        }
        ifs >> j;
        if (!ifs.eof() && ifs.fail()) {
            THROW_OR_ABORT("Error reading from file: \"" + filename + '"');
        }
        if (j.contains("variables")) {
            for (const auto& [key, value] : j["variables"].get<std::map<std::string, std::string>>()) {
                result.variables.insert(key, value);
            }
        }
        if (j.contains("requires")) {
            result.requires_ = j["requires"].get<std::vector<std::string>>();
        }
        result.name = j.at("name");
        if (j.contains("on_init")) {
            result.on_init = j["on_init"].get<std::vector<std::string>>();
        }
    } catch (const nlohmann::json::exception& e) {
        throw std::runtime_error("Error loading file \"" + filename + "\": " + e.what());
    }
    return result;
}
