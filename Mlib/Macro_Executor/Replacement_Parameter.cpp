#include "Replacement_Parameter.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Json/Misc.hpp>
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

void expression_from_json(const nlohmann::json& j, std::vector<std::string>& expression) {
    if (j.type() != nlohmann::detail::value_t::array) {
        expression.resize(1);
        j.get_to(expression[0]);
    } else {
        j.get_to(expression);
    }
}

void expression_from_json(const nlohmann::json& j, std::vector<std::vector<std::string>>& expression) {
    if (j.type() != nlohmann::detail::value_t::array) {
        expression.resize(1);
        expression[0].resize(1);
        j.get_to(expression[0][0]);
    } else {
        auto v = j.get<std::vector<nlohmann::json>>();
        expression.resize(v.size());
        for (size_t i = 0; i < v.size(); ++i) {
            expression_from_json(v[i], expression[i]);
        }
    }
}

void Mlib::from_json(const nlohmann::json& j, ReplacementParameterRequired& rp) {
    validate(j, KnownRequired::options);
    expression_from_json(j.at(KnownRequired::fixed), rp.fixed);
    expression_from_json(j.at(KnownRequired::dynamic), rp.dynamic);
    rp.focus_mask = j.contains(KnownRequired::focus_mask)
        ? focus_from_string(j.at(KnownRequired::focus_mask).get<std::string>())
        : Focus::ALWAYS;
}

void Mlib::from_json(const nlohmann::json& j, ReplacementParameter& rp) {
    validate(j, KnownArgs::options);
    j.at(KnownArgs::id).get_to(rp.id);
    j.at(KnownArgs::title).get_to(rp.title);
    if (j.contains(KnownArgs::required)) {
        j.at(KnownArgs::required).get_to(rp.required);
    }
    if (j.contains(KnownArgs::database)) {
        rp.database.merge(JsonMacroArguments{j.at(KnownArgs::database)});
    }
    if (j.contains(KnownArgs::on_init)) {
        j.at(KnownArgs::on_init).get_to(rp.on_init);
    }
    if (j.contains(KnownArgs::on_before_select)) {
        j.at(KnownArgs::on_before_select).get_to(rp.on_before_select);
    }
    if (j.contains(KnownArgs::on_execute)) {
        j.at(KnownArgs::on_execute).get_to(rp.on_execute);
    }
}
