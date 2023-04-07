#pragma once
#include <Mlib/Json.hpp>
#include <iosfwd>
#include <list>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace Mlib {

class JsonMacroArguments {
    friend std::ostream& operator << (std::ostream& ostr, const JsonMacroArguments& arguments);
public:
    void insert_json(const nlohmann::json& j);
    void insert_json(const std::string& key, const nlohmann::json& j);
    void insert_path(const std::string& key, const std::string& value);
    void insert_path_list(const std::string& key, const std::list<std::string>& value);
    template <class T>
    std::optional<T> try_get(const std::string& name) const {
        return j_.contains(name)
            ? j_.at(name).get<T>()
            : std::nullopt;
    }
    bool contains_json(const std::string& name) const;
    nlohmann::json at(const std::string& name) const;
    template <class T>
    T get(const std::string& name) const {
        return j_.at(name).get<T>();
    }
    template <class T>
    T get(const std::string& name, const T& default_) const {
        return j_.contains(name)
            ? j_.at(name).get<T>()
            : default_;
    }
    template <class TData, class TOperation>
    auto get_vector(const std::string& name, const TOperation& op) const {
        std::vector<decltype(op(j_.get<TData>()))> result;
        result.reserve(j_.size());
        for (auto& e : j_.at(name)) {
            result.push_back(op(e));
        }
        return result;
    }
    const std::string& path(const std::string& name) const;
    const std::list<std::string>& path_list(const std::string& name) const;
    bool contains_path(const std::string& name) const;
    bool contains_path_list(const std::string& name) const;
    void validate(const std::set<std::string>& allowed_attributes) const;
private:
    nlohmann::json j_;
    std::map<std::string, std::string> pathes_;
    std::map<std::string, std::list<std::string>> path_lists_;
};

std::ostream& operator << (std::ostream& ostr, const JsonMacroArguments& arguments);

}
