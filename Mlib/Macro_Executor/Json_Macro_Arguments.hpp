#pragma once
#include <Mlib/FPath.hpp>
#include <Mlib/Json/Json_View.hpp>
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

struct FPath;

class JsonMacroArguments: public JsonView {
public:
    JsonMacroArguments();
    explicit JsonMacroArguments(nlohmann::json j);
    void set(const std::string& key, nlohmann::json value);
    void merge(const JsonMacroArguments& other, const std::string& prefix="");
    void insert_json(const nlohmann::json& j);
    void insert_json(const std::string& key, nlohmann::json j);
    void set_fpathes(const std::function<std::list<std::string>(const std::filesystem::path& f)>& fpathes);
    void set_fpath(const std::function<FPath(const std::filesystem::path& f)>& fpath);
    void set_spath(const std::function<std::string(const std::filesystem::path& f)>& spath);
    std::string get_multiline_string() const;
    std::string at_multiline_string(const std::string& name) const;
    std::string at_multiline_string(const std::string& name, const std::string& default_) const;
    std::string path(const std::string& name) const;
    std::string path(const std::string& name, const std::string& deflt) const;
    FPath path_or_variable(const std::string& name) const;
    FPath try_path_or_variable(const std::string& name) const;
    std::vector<FPath> pathes_or_variables(const std::string& name) const;
    template <class TOperation>
    auto pathes_or_variables(const std::string& name, const TOperation& op) const {
        return Mlib::get_vector<std::string>(j_.at(name), [this, &op](const std::string& s){return op(fpath_(s));});
    }
    std::list<std::string> path_list(const std::string& name) const;
    std::vector<JsonMacroArguments> elements() const;
    template <class TOperation>
    auto elements(const TOperation& op) const {
        return Mlib::get_vector<nlohmann::json>(j_, [this, &op](const nlohmann::json& c){return op(as_child(c));});
    }
    JsonMacroArguments child(const std::string& name) const;
    std::optional<JsonMacroArguments> try_get_child(const std::string& name) const;
    nlohmann::json subst_and_replace(const nlohmann::json& j) const;;
    void validate(const std::set<std::string>& allowed_attributes, const std::string& prefix = "") const;
private:
    JsonMacroArguments as_child(const nlohmann::json& j) const;
    std::function<std::list<std::string>(const std::filesystem::path& f)> fpathes_;
    std::function<FPath(const std::filesystem::path& f)> fpath_;
    std::function<std::string(const std::filesystem::path& f)> spath_;
};

}
