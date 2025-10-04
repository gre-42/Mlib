#include "Replacement_Parameter.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Json/Json_View.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

using namespace Mlib;

namespace KnownRequired {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(fixed);
DECLARE_ARGUMENT(dynamic);
DECLARE_ARGUMENT(focus_mask);
}

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(id);
DECLARE_ARGUMENT(title);
DECLARE_ARGUMENT(database);
DECLARE_ARGUMENT(required);
DECLARE_ARGUMENT(on_init);
DECLARE_ARGUMENT(on_before_select);
DECLARE_ARGUMENT(on_execute);
}

ReplacementParameterAndFilename ReplacementParameterAndFilename::from_json(const std::string& filename) {
    try {
        nlohmann::json j;
        auto ifs_p = create_ifstream(filename);
        auto& ifs = *ifs_p;
        if (ifs.fail()) {
            THROW_OR_ABORT("Could not open replacement parameter entry file \"" + filename + '"');
        }
        ifs >> j;
        if (!ifs.eof() && ifs.fail()) {
            THROW_OR_ABORT("Error reading from file: \"" + filename + '"');
        }
        return ReplacementParameterAndFilename{
            .rp = j.get<ReplacementParameter>(),
            .filename = filename};
    } catch (const nlohmann::json::exception& e) {
        throw std::runtime_error("Error loading file \"" + filename + "\": " + e.what());
    }
}

void Mlib::from_json(const nlohmann::json& j, ReplacementParameterRequired& rp) {
    JsonView jv{ j };
    jv.validate(KnownRequired::options);
    expression_from_json(jv.at(KnownRequired::fixed), rp.fixed);
    expression_from_json(jv.at(KnownRequired::dynamic), rp.dynamic);
    rp.focus_mask = jv.contains(KnownRequired::focus_mask)
        ? focus_from_string(jv.at(KnownRequired::focus_mask).get<std::string>())
        : Focus::ALWAYS;
}

void Mlib::from_json(const nlohmann::json& j, ReplacementParameter& rp) {
    JsonView jv{ j };
    jv.validate(KnownArgs::options);
    jv.at(KnownArgs::id).get_to(rp.id);
    jv.at(KnownArgs::title).get_to(rp.title);
    if (jv.contains(KnownArgs::required)) {
        jv.at(KnownArgs::required).get_to(rp.required);
    }
    if (jv.contains(KnownArgs::database)) {
        rp.database.merge(JsonMacroArguments{jv.at(KnownArgs::database)});
    }
    if (jv.contains(KnownArgs::on_init)) {
        jv.at(KnownArgs::on_init).get_to(rp.on_init);
    }
    if (jv.contains(KnownArgs::on_before_select)) {
        jv.at(KnownArgs::on_before_select).get_to(rp.on_before_select);
    }
    if (jv.contains(KnownArgs::on_execute)) {
        jv.at(KnownArgs::on_execute).get_to(rp.on_execute);
    }
}
