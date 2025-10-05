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

static const nlohmann::json* get_internal(
    const nlohmann::json& j,
    const std::vector<std::string>& keys,
    size_t rec)
{
    if (keys.empty()) {
        THROW_OR_ABORT("JSON contains called with empty path");
    }
    if (j.type() != nlohmann::detail::value_t::object) {
        THROW_OR_ABORT("JSON entry is not an object");
    }
    auto it = j.find(keys[rec]);
    if (it == j.end()) {
        return nullptr;
    }
    if (rec == keys.size() - 1) {
        return &it.value();
    }
    return get_internal(*it, keys, rec + 1);
}

bool JsonView::contains(const std::vector<std::string>& name) const {
    return get_internal(j_, name, 0) != nullptr;
}

bool JsonView::contains_non_null(const std::vector<std::string>& name) const {
    auto v = get_internal(j_, name, 0);
    return ((v == nullptr) && (v->type() != nlohmann::detail::value_t::null));
}

std::optional<nlohmann::json> JsonView::try_at(const std::vector<std::string>& name) const {
    auto v = get_internal(j_, name, 0);
    if (v == nullptr) {
        return std::nullopt;
    }
    return *v;
}

std::optional<nlohmann::json> JsonView::try_at_non_null(const std::vector<std::string>& name) const {
    auto v = get_internal(j_, name, 0);
    if ((v == nullptr) || (v->type() == nlohmann::detail::value_t::null)) {
        return std::nullopt;
    }
    return *v;
}

std::optional<nlohmann::json> JsonView::try_resolve(std::string_view key) const {
    return try_at(key);
}

std::optional<nlohmann::json> JsonView::try_at(std::string_view name) const {
    if (!j_.contains(name)) {
        return std::nullopt;
    }
    return j_.at(name);
}

std::optional<nlohmann::json> JsonView::try_at_non_null(std::string_view name) const {
    auto it = j_.find(name);
    if ((it == j_.end()) || (it.value().type() == nlohmann::detail::value_t::null)) {
        return std::nullopt;
    }
    return *it;
}

nlohmann::json JsonView::at(std::string_view name) const {
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
