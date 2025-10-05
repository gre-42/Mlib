#pragma once
#include <Mlib/Json/Json_Key.hpp>
#include <Mlib/Json/Misc.hpp>
#include <Mlib/Strings/Join_Arguments.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <functional>
#include <iosfwd>
#include <list>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace Mlib {

enum class CheckIsObjectBehavior {
    CHECK,
    NO_CHECK
};

class JsonView {
    friend std::ostream& operator << (std::ostream& ostr, const JsonView& container);
public:
    explicit JsonView(
        const nlohmann::json& j,
        CheckIsObjectBehavior check = CheckIsObjectBehavior::CHECK);
    std::optional<nlohmann::json> try_resolve(std::string_view key) const;
    template <class TKey0, class TKey1, class... TKeys2>
    std::optional<nlohmann::json> try_resolve(const TKey0& key0, const TKey1& key1, TKeys2&&... path) const {
        auto res0 = try_at(key0);
        if (res0.has_value()) {
            return JsonView{ res0.value() }.try_resolve(key1, std::forward<TKeys2>(path)...);
        } else {
            return std::nullopt;
        }
    }
    template <class... TKeys>
    nlohmann::json resolve(TKeys... path) const {
        auto res = try_resolve(path...);
        if (!res.has_value()) {
            THROW_OR_ABORT("Cannot resolve JSON-path \"" + join_arguments("/", path...) + '"');
        } else {
            return *res;
        }
    }
    template <class TValue, class... TKeys>
    TValue resolve(TKeys... path) const {
        auto res = try_resolve(path...);
        if (!res.has_value()) {
            THROW_OR_ABORT("Cannot resolve JSON-path \"" + join_arguments("/", path...) + '"');
        } else {
            return res->template get<TValue>();
        }
    }
    template <class TValue, class... TKeys>
    TValue resolve_default(const TValue& deflt, TKeys... path) const {
        auto res = try_resolve(path...);
        if (!res.has_value()) {
            return deflt;
        } else {
            return res->template get<TValue>();
        }
    }
    std::optional<nlohmann::json> try_at(std::string_view name) const;
    std::optional<nlohmann::json> try_at_non_null(std::string_view name) const;
    std::optional<nlohmann::json> try_at(const std::vector<std::string>& name) const;
    std::optional<nlohmann::json> try_at_non_null(const std::vector<std::string>& name) const;
    template <class T, JsonKey Key>
    std::optional<T> try_at(const Key& name) const {
        if (!contains(name)) {
            return std::nullopt;
        }
        return at<T>(name);
    }
    template <class T, JsonKey Key>
    std::optional<T> try_at_non_null(const Key& name) const {
        if (!contains_non_null(name)) {
            return std::nullopt;
        }
        return at<T>(name);
    }
    bool contains(std::string_view name) const;
    bool contains_non_null(std::string_view name) const;
    bool contains(const std::vector<std::string>& name) const;
    bool contains_non_null(const std::vector<std::string>& name) const;
    template <class T>
    T get() const {
        return j_.get<T>();
    }
    template <class TData, class TOperation>
    auto get_vector(const TOperation& op) const {
        return Mlib::get_vector<TData>(j_, op);
    }
    nlohmann::json at(std::string_view name) const;
    nlohmann::json at(const std::vector<std::string>& path) const;
    template <class T, JsonKey Key>
    T at(const Key& name) const {
        if (j_.type() == nlohmann::detail::value_t::null) {
            THROW_OR_ABORT("Attempt to retrieve value for key on null object: \"" + std::string{ name } + '"');
        }
        auto o = at(name);
        try {
            return json_get<T>(o);
        } catch (const std::runtime_error& e) {
            throw std::runtime_error("Runtime error interpreting key \"" + std::string{ name } + "\": " + e.what());
        } catch (const nlohmann::json::type_error& e) {
            throw std::runtime_error("Incorrect type for key \"" + std::string{ name } + "\": " + e.what());
        } catch (const std::out_of_range& e) {
            throw std::runtime_error("Error retrieving key \"" + std::string{ name } + "\": " + e.what());
        }
    }
    template <class T, JsonKey Key>
    T at(const Key& name, const T& default_) const {
        return j_.contains(name)
            ? at<T>(name)
            : default_;
    }
    template <class T, JsonKey Key>
    T at_non_null(const Key& name, const T& default_) const {
        return contains_non_null(name)
            ? at<T>(name)
            : default_;
    }
    template <class TData, JsonKey Key>
    auto try_at_vector(const Key& name) const {
        return contains_non_null(name)
            ? at<std::vector<TData>>(name)
            : std::vector<TData>();
    }
    template <class TData, class TOperation, JsonKey Key>
    auto at_vector(const Key& name, const TOperation& op) const {
        const auto& val = j_.at(name);
        if (val.type() != nlohmann::detail::value_t::array) {
            THROW_OR_ABORT("Type is not array for key \"" + std::string{ name } + '"');
        }
        return Mlib::get_vector<TData>(val, op);
    }
    template <class TData, class TOperation, JsonKey Key>
    auto try_at_vector(const Key& name, const TOperation& op) const {
        if (!contains_non_null(name)) {
            using TRet = decltype(at_vector<TData>(name, op));
            return TRet();
        }
        return at_vector<TData>(name, op);
    }
    template <class TData, class TOperation, JsonKey Key>
    auto at_vector_non_null(const Key& name, const TOperation& op) const {
        const auto& val = j_.at(name);
        if ((val.type() != nlohmann::detail::value_t::array) &&
            (val.type() != nlohmann::detail::value_t::null))
        {
            THROW_OR_ABORT("Type is not array or null for key \"" + std::string{ name } + '"');
        }
        return Mlib::get_vector_non_null<TData>(val, op);
    }
    template <class TData, class TOperation, JsonKey Key>
    auto at_vector_non_null_optional(const Key& name, const TOperation& op) const {
        if (!j_.contains(name)) {
            return decltype(at_vector_non_null<TData>(name, op))();
        }
        return at_vector_non_null<TData>(name, op);
    }
    inline void validate(const std::set<std::string_view>& known_keys, std::string_view prefix = "") const {
        Mlib::validate(j_, known_keys, prefix);
    }
    inline void validate_complement(const std::set<std::string_view>& known_keys, std::string_view prefix = "") const {
        Mlib::validate_complement(j_, known_keys, prefix);
    }
    inline const nlohmann::json& json() const {
        return j_;
    }
private:
    const nlohmann::json& j_;
};

std::ostream& operator << (std::ostream& ostr, const JsonView& view);

}
