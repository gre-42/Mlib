#pragma once
#include <Mlib/Json/Misc.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <functional>
#include <iosfwd>
#include <list>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace Mlib {

class JsonView {
    friend std::ostream& operator << (std::ostream& ostr, const JsonView& container);
public:
    explicit JsonView(const nlohmann::json& j);
    std::optional<nlohmann::json> try_at(const std::string& name) const;
    template <class T>
    std::optional<T> try_at(const std::string& name) const {
        return j_.contains(name)
            ? j_.at(name).get<T>()
            : std::optional<T>();
    }
    bool contains(const std::string& name) const;
    bool contains_non_null(const std::string& name) const;
    template <class T>
    T get() const {
        return j_.get<T>();
    }
    template <class TData, class TOperation>
    auto get_vector(const TOperation& op) const {
        return Mlib::get_vector<TData>(j_, op);
    }
    nlohmann::json at(const std::string& name) const;
    template <class T>
    T at(const std::string& name) const {
        if (j_.type() == nlohmann::detail::value_t::null) {
            THROW_OR_ABORT("Attempt to retrieve value for key on null object: \"" + name + '"');
        }
        try {
            return json_get<T>(j_.at(name));
        } catch (const std::runtime_error& e) {
            throw std::runtime_error("Runtime error interpreting key \"" + name + "\": " + e.what());
        } catch (const nlohmann::json::type_error& e) {
            throw std::runtime_error("Incorrect type for key \"" + name + "\": " + e.what());
        } catch (const std::out_of_range& e) {
            throw std::runtime_error("Error retrieving key \"" + name + "\": " + e.what());
        }
    }
    template <class T>
    T at(const std::string& name, const T& default_) const {
        return j_.contains(name)
            ? at<T>(name)
            : default_;
    }
    template <class T>
    T at_non_null(const std::string& name, const T& default_) const {
        return (j_.at(name).type() != nlohmann::detail::value_t::null)
            ? at<T>(name)
            : default_;
    }
    template <class TData, class TOperation>
    auto at_vector(const std::string& name, const TOperation& op) const {
        auto val = j_.at(name);
        if (val.type() != nlohmann::detail::value_t::array) {
            THROW_OR_ABORT("Type is not array for key \"" + name + '"');
        }
        return Mlib::get_vector<TData>(val, op);
    }
    template <class TData, class TOperation>
    auto at_vector_non_null(const std::string& name, const TOperation& op) const {
        auto val = j_.at(name);
        if ((val.type() != nlohmann::detail::value_t::array) &&
            (val.type() != nlohmann::detail::value_t::null))
        {
            THROW_OR_ABORT("Type is not array or null for key \"" + name + '"');
        }
        return Mlib::get_vector_non_null<TData>(val, op);
    }
    template <class TData, class TOperation>
    auto at_vector_non_null_optional(const std::string& name, const TOperation& op) const {
        if (!j_.contains(name)) {
            return decltype(at_vector_non_null<TData>(name, op))();
        }
        return at_vector_non_null<TData>(name, op);
    }
    inline nlohmann::detail::value_t type() const {
        return j_.type();
    }
    inline void validate(const std::set<std::string>& known_keys, const std::string& prefix = "") const {
        Mlib::validate(j_, known_keys, prefix);
    }
    inline const nlohmann::json& json() const {
        return j_;
    }
private:
    const nlohmann::json& j_;
};

std::ostream& operator << (std::ostream& ostr, const JsonView& view);

}