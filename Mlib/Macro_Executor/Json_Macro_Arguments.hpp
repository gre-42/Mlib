#pragma once
#include <Mlib/Json.hpp>
#include <Mlib/Strings/To_Number.hpp>
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
    JsonMacroArguments();
    explicit JsonMacroArguments(nlohmann::json j);
    void insert_json(const nlohmann::json& j);
    void insert_json(const std::string& key, nlohmann::json j);
    void insert_path(const std::string& key, std::string value);
    void insert_path_list(const std::string& key, std::list<std::string> value);
    void insert_child(const std::string& key, JsonMacroArguments child);
    template <class T>
    std::optional<T> try_at(const std::string& name) const {
        return j_.contains(name)
            ? j_.at(name).get<T>()
            : std::nullopt;
    }
    bool contains_json(const std::string& name) const;
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
        return json_get<T>(j_.at(name));
    }
    template <class T>
    T at(const std::string& name, const T& default_) const {
        return j_.contains(name)
            ? at<T>(name)
            : default_;
    }
    template <class TData, class TOperation>
    auto at_vector(const std::string& name, const TOperation& op) const {
        return Mlib::get_vector<TData>(j_.at(name), op);
    }
    const std::string& path(const std::string& name) const;
    const std::list<std::string>& path_list(const std::string& name) const;
    const JsonMacroArguments& child(const std::string& name) const;
    const JsonMacroArguments* try_get_child(const std::string& name) const;
    bool contains_path(const std::string& name) const;
    bool contains_path_list(const std::string& name) const;
    bool contains_child(const std::string& name) const;
    void validate(const std::set<std::string>& allowed_attributes) const;
private:
    nlohmann::json j_;
    std::map<std::string, std::string> pathes_;
    std::map<std::string, std::list<std::string>> path_lists_;
    std::map<std::string, JsonMacroArguments> children_;
};

std::ostream& operator << (std::ostream& ostr, const JsonMacroArguments& arguments);

}
