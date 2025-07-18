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
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace Mlib {

struct FPath;
class AssetReferences;

enum class SubstitutionMode {
    DEFAULT,
    ARGUMENT_COMPATIBILITY
};

namespace Filter {
constexpr struct With {} with;
constexpr struct Without {} without;
}

class JsonMacroArguments: public JsonView {
public:
    JsonMacroArguments();
    JsonMacroArguments(const JsonMacroArguments& other);
    JsonMacroArguments(JsonMacroArguments&& other) noexcept;
    explicit JsonMacroArguments(nlohmann::json j);
    JsonMacroArguments(const nlohmann::json& j, Filter::With, const std::set<std::string>& with);
    JsonMacroArguments(const nlohmann::json& j, Filter::Without, const std::set<std::string>& without);
    ~JsonMacroArguments();
    void set(std::string_view key, nlohmann::json value);
    void merge(const JsonMacroArguments& other, std::string_view prefix="");
    void insert_json(const nlohmann::json& j);
    void insert_json(const nlohmann::json& j, Filter::With, const std::set<std::string>& with);
    void insert_json(const nlohmann::json& j, Filter::Without, const std::set<std::string>& without);
    void insert_json(std::string_view key, nlohmann::json j);
    void set_fpathes(std::function<std::list<std::string>(const std::filesystem::path& f)> fpathes);
    void set_fpath(std::function<FPath(const std::filesystem::path& f)> fpath);
    void set_spath(std::function<std::string(const std::filesystem::path& f)> spath);
    std::list<std::string> fpathes(const std::filesystem::path& f) const;
    FPath fpath(const std::filesystem::path& f) const;
    std::string spath(const std::filesystem::path& f) const;
    std::string get_multiline_string() const;
    std::string at_multiline_string(std::string_view name) const;
    std::string at_multiline_string(std::string_view name, std::string_view default_) const;
    std::string path(std::string_view name) const;
    std::string path(std::string_view name, std::string_view deflt) const;
    FPath path_or_variable(std::string_view name) const;
    FPath try_path_or_variable(std::string_view name) const;
    std::vector<FPath> pathes_or_variables(std::string_view name) const;
    std::vector<FPath> try_pathes_or_variables(std::string_view name) const;
    template <class TOperation>
    auto pathes_or_variables(std::string_view name, const TOperation& op) const {
        auto el = at(name);
        if (el.type() != nlohmann::detail::value_t::array) {
            THROW_OR_ABORT("Not an array: \"" + std::string{ name } + '"');
        }
        return Mlib::get_vector<std::string>(el, [this, &op](std::string_view s){return op(fpath_(s));});
    }
    std::list<std::string> path_list(std::string_view name) const;
    std::string spath(std::string_view name) const;
    std::vector<JsonMacroArguments> children(std::string_view name) const;
    template <class TOperation>
    auto children(std::string_view name, const TOperation& op) const {
        auto el = at(name);
        if (el.type() != nlohmann::detail::value_t::array) {
            THROW_OR_ABORT("Not an array: \"" + std::string{ name } + '"');
        }
        try {
            return Mlib::get_vector<nlohmann::json>(el, [this, &op](const nlohmann::json& c){return op(as_child(c));});
        } catch (const std::runtime_error& e) {
            throw std::runtime_error("Could not interpret \"" + std::string{ name } + "\" as a child array: " + e.what());
        }
    }
    template <class TData, class TOperation>
    std::vector<TData> children(
        std::string_view name,
        const TOperation& op,
        std::vector<TData> deflt) const
    {
        if (!contains(name)) {
            return deflt;
        } else {
            return children(name, op);
        }
    }
    JsonMacroArguments child(std::string_view name) const;
    std::optional<JsonMacroArguments> try_get_child(std::string_view name) const;
    nlohmann::json subst_and_replace(
        const nlohmann::json& j,
        const nlohmann::json& globals,
        const AssetReferences& asset_references,
        SubstitutionMode mode) const;
    JsonMacroArguments as_child(const nlohmann::json& j) const;
    inline nlohmann::json&& move_json() {
        return std::move(j_);
    }
    inline decltype(auto) items() const {
        return j_.items();
    }
private:
    nlohmann::json j_;
    std::function<std::list<std::string>(const std::filesystem::path& f)> fpathes_;
    std::function<FPath(const std::filesystem::path& f)> fpath_;
    std::function<std::string(const std::filesystem::path& f)> spath_;
};

}
