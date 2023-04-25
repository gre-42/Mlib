#include "Replacement_Parameter.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Json.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(variables);
DECLARE_ARGUMENT(required);
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(on_init);
}

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
        validate(j, KnownArgs::options);
        if (j.contains(KnownArgs::variables)) {
            for (const auto& [key, value] : j[KnownArgs::variables].items()) {
                result.variables.insert_json(key, value);
            }
        }
        if (j.contains(KnownArgs::required)) {
            result.requires_ = j[KnownArgs::required].get<std::vector<std::string>>();
        }
        result.name = j.at(KnownArgs::name);
        if (j.contains(KnownArgs::on_init)) {
            result.on_init = j[KnownArgs::on_init].get<std::vector<nlohmann::json>>();
        }
    } catch (const nlohmann::json::exception& e) {
        throw std::runtime_error("Error loading file \"" + filename + "\": " + e.what());
    }
    return result;
}
