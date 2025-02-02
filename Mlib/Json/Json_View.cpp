#include "Json_View.hpp"
#include <ostream>

using namespace Mlib;

JsonView::JsonView(
    const nlohmann::json& j,
    CheckIsObjectBehavior check)
: j_{j}
{
    if ((check == CheckIsObjectBehavior::CHECK) &&
        (j_.type() != nlohmann::detail::value_t::object))
    {
        THROW_OR_ABORT("JSON is not of type object");
    }
}

bool JsonView::contains(std::string_view name) const {
    return j_.contains(name);
}

bool JsonView::contains_non_null(std::string_view name) const {
    return j_.contains(name) &&
           (j_.at(name).type() != nlohmann::detail::value_t::null);
}

nlohmann::json JsonView::try_resolve() const {
    return (const nlohmann::json&)(*this);
}

std::optional<nlohmann::json> JsonView::try_resolve(std::string_view key) const {
    return try_at(key);
}

std::optional<nlohmann::json> JsonView::try_at(std::string_view name) const {
    return j_.contains(name)
        ? j_.at(name)
        : std::optional<nlohmann::json>();
}

std::optional<nlohmann::json> JsonView::try_at_non_null(std::string_view name) const {
    auto it = j_.find(name);
    if ((it == j_.end()) || (it.value().type() == nlohmann::detail::value_t::null)) {
        return std::nullopt;
    }
    return *it;
}

nlohmann::json JsonView::at(const std::string_view& name) const {
    auto it = j_.find(name);
    if (it == j_.end()) {
        THROW_OR_ABORT("Cannot find key with name \"" + std::string{ name } + "\"");
    }
    return *it;
}

std::ostream& Mlib::operator << (std::ostream& ostr, const JsonView& view) {
    ostr << "JSON: " << view.j_;
    return ostr;
}
