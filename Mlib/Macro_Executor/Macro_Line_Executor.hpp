#pragma once
#include <Mlib/Json.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Regex.hpp>
#include <filesystem>
#include <functional>
#include <list>
#include <map>
#include <string>

namespace Mlib {

class MacroRecorder;
class SubstitutionMap;
class NotifyingSubstitutionMap;
struct FPath;

class MacroLineExecutor {
    friend MacroRecorder;
public:
    typedef std::function<bool(
        const std::string& context,
        const MacroLineExecutor& macro_line_executor,
        const std::string& name,
        const JsonMacroArguments& arguments)> JsonUserFunction;
    typedef std::function<bool(
        const std::string& context,
        const std::function<std::string(const std::string&)>& spath,
        const std::function<FPath(const std::string&)>& fpath,
        const std::function<std::list<std::string>(const std::string&)>& fpathes,
        const MacroLineExecutor& macro_line_executor,
        const std::string& line,
        SubstitutionMap* local_substitutions)> UserFunction;
    MacroLineExecutor(
        MacroRecorder& macro_recorder,
        std::string script_filename,
        const std::list<std::string>* search_path,
        JsonUserFunction json_user_function,
        UserFunction user_function,
        std::string context,
        const NotifyingSubstitutionMap& global_substitutions,
        bool verbose);
    MacroLineExecutor changed_script_filename(
        std::string script_filename) const;
    void operator () (
        const nlohmann::json& j,
        SubstitutionMap* local_substitutions,
        const JsonMacroArguments* caller_args) const;
    void operator () (
        const std::string& line,
        SubstitutionMap* local_substitutions) const;
    std::string substitute_globals(const std::string& str) const;
private:
    std::list<std::string> fpathes(const std::filesystem::path& f) const;
    FPath fpath(const std::filesystem::path& f) const;
    std::string spath(const std::filesystem::path& f) const;

    MacroRecorder& macro_recorder_;
    std::string script_filename_;
    const std::list<std::string>* search_path_;
    JsonUserFunction json_user_function_;
    UserFunction user_function_;
    std::string context_;
    const NotifyingSubstitutionMap& global_substitutions_;
    bool verbose_;
};

}
