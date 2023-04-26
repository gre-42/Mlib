#pragma once
#include <Mlib/FPath.hpp>
#include <Mlib/Json.hpp>
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

class JsonMacroArguments {
    friend std::ostream& operator << (std::ostream& ostr, const JsonMacroArguments& arguments);
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
        return Mlib::get_vector<TData>(j_.at(name), op);
    }
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
    void validate(const std::set<std::string>& allowed_attributes) const;
private:
    JsonMacroArguments as_child(const nlohmann::json& j) const;
    nlohmann::json j_;
    std::function<std::list<std::string>(const std::filesystem::path& f)> fpathes_;
    std::function<FPath(const std::filesystem::path& f)> fpath_;
    std::function<std::string(const std::filesystem::path& f)> spath_;
};

std::ostream& operator << (std::ostream& ostr, const JsonMacroArguments& arguments);

}
