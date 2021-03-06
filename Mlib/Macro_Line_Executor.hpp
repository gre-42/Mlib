#pragma once
#include <Mlib/Regex.hpp>
#include <functional>
#include <list>
#include <map>
#include <string>

namespace Mlib {

class MacroRecorder;
class SubstitutionMap;
struct FPath;

class MacroLineExecutor {
    friend MacroRecorder;
public:
    typedef std::function<bool(
        const std::string& context,
        const std::function<FPath(const std::string&)>& fpath,
        const MacroLineExecutor& macro_line_executor,
        const std::string& line,
        SubstitutionMap* local_substitutions)> UserFunction;
    MacroLineExecutor(
        MacroRecorder& macro_file_executor,
        const std::string& script_filename,
        const std::list<std::string>& search_path,
        const UserFunction& user_function,
        const std::string& context,
        const SubstitutionMap& global_substitutions,
        bool verbose);
    void operator () (
        const std::string& line,
        SubstitutionMap* local_substitutions,
        const RegexSubstitutionCache& rsc) const;
    std::string substitude_globals(const std::string& str, const RegexSubstitutionCache& rsc) const;
private:
    MacroRecorder& macro_file_executor_;
    std::string script_filename_;
    std::list<std::string> search_path_;
    UserFunction user_function_;
    std::string context_;
    const SubstitutionMap& global_substitutions_;
    bool verbose_;
};

}
