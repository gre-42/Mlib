#pragma once
#include <Mlib/Regex.hpp>
#include <functional>
#include <list>
#include <map>
#include <string>

namespace Mlib {

class SubstitutionString;
class MacroFileExecutor;

class MacroLineExecutor {
    friend MacroFileExecutor;
public:
    typedef std::function<bool(
        const std::string& context,
        const std::function<std::string(const std::string&)>& fpath,
        const MacroLineExecutor& macro_line_executor,
        const std::string& line)> UserFunction;
    MacroLineExecutor(
        MacroFileExecutor& macro_file_executor,
        const std::string& script_filename,
        const std::string& working_directory,
        const UserFunction& user_function,
        const std::string& context,
        const SubstitutionString& substitutions,
        bool verbose);
    void operator () (
        const std::string& line,
        const RegexSubstitutionCache& rsc) const;
private:
    MacroFileExecutor& macro_file_executor_;
    std::string script_filename_;
    std::string working_directory_;
    UserFunction user_function_;
    std::string context_;
    SubstitutionString substitutions_;
    bool verbose_;
};

}
