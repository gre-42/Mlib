#pragma once
#include <Mlib/Json.hpp>
#include <iosfwd>
#include <list>
#include <map>
#include <optional>
#include <string>

namespace Mlib {

class JsonMacroArguments {
    friend std::ostream& operator << (std::ostream& ostr, const JsonMacroArguments& arguments);
public:
    void insert_json(const nlohmann::json& j);
    void insert_script(const std::string& key, const std::string& value);
    void insert_path(const std::string& key, const std::string& value);
    void insert_path_list(const std::string& key, const std::list<std::string>& value);
    template <class T>
    std::optional<T> try_get(const std::string& name) const {
        return j_.contains(name)
            ? j_.at(name).get<T>()
            : std::nullopt;
    }
    bool contains_json(const std::string& name) const;
    template <class T>
    T get(const std::string& name) const {
        return j_.at(name).get<T>();
    }
    const std::string& script(const std::string& name) const;
    const std::string& path(const std::string& name) const;
    const std::list<std::string>& path_list(const std::string& name) const;
private:
    nlohmann::json j_;
    std::map<std::string, std::string> scripts_;
    std::map<std::string, std::string> pathes_;
    std::map<std::string, std::list<std::string>> path_lists_;
};

std::ostream& operator << (std::ostream& ostr, const JsonMacroArguments& arguments);

}
