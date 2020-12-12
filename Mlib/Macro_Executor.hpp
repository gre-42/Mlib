#pragma once
#include <Mlib/Regex.hpp>
#include <functional>
#include <list>
#include <map>
#include <string>

namespace Mlib {

class MacroLineExecutor;
class SubstitutionString;

struct Macro {
    std::string filename;
    std::list<std::string> lines;
};

class MacroFileExecutor {
    friend MacroLineExecutor;
public:
    typedef std::function<bool(
        const std::function<std::string(const std::string&)>& fpath,
        const MacroLineExecutor& macro_line_executor,
        const std::string& line)> UserFunction;
    void operator () (const MacroLineExecutor& macro_line_executor);
private:
    std::map<std::string, Macro> macros_;
};

class MacroLineExecutor {
    friend MacroFileExecutor;
public:
    MacroLineExecutor(
        MacroFileExecutor& macro_file_executor,
        const std::string& script_filename,
        const std::string& working_directory,
        const MacroFileExecutor::UserFunction& execute_user_function,
        const SubstitutionString& substitutions,
        bool verbose);
    void operator () (const std::string& line) const;
private:
    MacroFileExecutor& macro_file_executor_;
    std::string script_filename_;
    std::string working_directory_;
    MacroFileExecutor::UserFunction execute_user_function_;
    SubstitutionString substitutions_;
    bool verbose_;
};

}
