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
class NotifyingJsonMacroArguments;
struct FPath;

class MacroLineExecutor {
    friend MacroRecorder;
public:
    typedef std::function<bool(
        const std::string& context,
        const MacroLineExecutor& macro_line_executor,
        const std::string& name,
        const JsonMacroArguments& arguments,
        JsonMacroArguments* local_json_macro_arguments)> JsonUserFunction;
    MacroLineExecutor(
        MacroRecorder& macro_recorder,
        std::string script_filename,
        const std::list<std::string>* search_path,
        JsonUserFunction json_user_function,
        std::string context,
        const NotifyingJsonMacroArguments& global_json_macro_arguments,
        bool verbose);
    MacroLineExecutor changed_script_filename(
        std::string script_filename) const;
    void operator () (
        const nlohmann::json& j,
        const JsonMacroArguments* caller_args,
        JsonMacroArguments* local_json_macro_arguments) const;
private:
    std::list<std::string> fpathes(const std::filesystem::path& f) const;
    FPath fpath(const std::filesystem::path& f) const;
    std::string spath(const std::filesystem::path& f) const;

    MacroRecorder& macro_recorder_;
    std::string script_filename_;
    const std::list<std::string>* search_path_;
    JsonUserFunction json_user_function_;
    std::string context_;
    const NotifyingJsonMacroArguments& global_json_macro_arguments_;
    bool verbose_;
};

}
