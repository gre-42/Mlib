#include "Json_View.hpp"
#include <ostream>

using namespace Mlib;

JsonView::JsonView(const nlohmann::json& j)
: j_{j}
{}

bool JsonView::contains(const std::string& name) const {
    return j_.contains(name);
}

bool JsonView::contains_non_null(const std::string& name) const {
    return j_.contains(name) &&
           (j_.at(name).type() != nlohmann::detail::value_t::null);
}

std::optional<nlohmann::json> JsonView::try_at(const std::string& name) const {
    return j_.contains(name)
        ? j_.at(name)
        : std::optional<nlohmann::json>();
}

nlohmann::json JsonView::at(const std::string& name) const {
    return j_.at(name);
}

std::ostream& Mlib::operator << (std::ostream& ostr, const JsonView& view) {
    ostr << "JSON: " << view.j_ << '\n';
    return ostr;
}
