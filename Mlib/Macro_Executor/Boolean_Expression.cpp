#include "Boolean_Expression.hpp"
#include <nlohmann/json.hpp>

using namespace Mlib;

static void expression_from_json(const nlohmann::json& j, std::vector<std::string>& expression) {
    if (j.type() != nlohmann::detail::value_t::array) {
        expression.resize(1);
        j.get_to(expression[0]);
    } else {
        j.get_to(expression);
    }
}

void Mlib::expression_from_json(const nlohmann::json& j, std::vector<std::vector<std::string>>& expression) {
    if (j.type() != nlohmann::detail::value_t::array) {
        expression.resize(1);
        expression[0].resize(1);
        j.get_to(expression[0][0]);
    } else {
        auto v = j.get<std::vector<nlohmann::json>>();
        expression.resize(v.size());
        for (size_t i = 0; i < v.size(); ++i) {
            ::expression_from_json(v[i], expression[i]);
        }
    }
}
