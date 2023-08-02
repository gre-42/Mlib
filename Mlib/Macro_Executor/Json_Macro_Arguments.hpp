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
class AssetReferences;

class JsonMacroArguments: public JsonView {
public:
    JsonMacroArguments();
    JsonMacroArguments(const JsonMacroArguments& other);
    JsonMacroArguments(JsonMacroArguments&& other);
    explicit JsonMacroArguments(nlohmann::json j);
    ~JsonMacroArguments();
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
        auto el = at(name);
        if (el.type() != nlohmann::detail::value_t::array) {
            THROW_OR_ABORT("Not an array: \"" + name + '"');
        }
        return Mlib::get_vector<std::string>(el, [this, &op](const std::string& s){return op(fpath_(s));});
    }
    std::list<std::string> path_list(const std::string& name) const;
    std::string spath(const std::string& name) const;
    std::vector<JsonMacroArguments> children(const std::string& name) const;
    template <class TOperation>
    auto children(const std::string& name, const TOperation& op) const {
        auto el = at(name);
        if (el.type() != nlohmann::detail::value_t::array) {
            THROW_OR_ABORT("Not an array: \"" + name + '"');
        }
        return Mlib::get_vector<nlohmann::json>(el, [this, &op](const nlohmann::json& c){return op(as_child(c));});
    }
    JsonMacroArguments child(const std::string& name) const;
    std::optional<JsonMacroArguments> try_get_child(const std::string& name) const;
    nlohmann::json subst_and_replace(
        const nlohmann::json& j,
        const nlohmann::json& globals,
        const AssetReferences& asset_references) const;
private:
    nlohmann::json j_;
    JsonMacroArguments as_child(const nlohmann::json& j) const;
    std::function<std::list<std::string>(const std::filesystem::path& f)> fpathes_;
    std::function<FPath(const std::filesystem::path& f)> fpath_;
    std::function<std::string(const std::filesystem::path& f)> spath_;
};

}
