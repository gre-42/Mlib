#pragma once
#include <functional>
#include <list>
#include <map>
#include <string>

namespace Mlib {

class SubstitutionString;

struct Macro {
    std::string filename;
    std::list<std::string> lines;
};

class MacroExecutor {
public:
    typedef std::function<bool(
        const std::function<std::string(const std::string&)>& fpath,
        const std::string& line)> UserFunction;
    void operator () (
        const std::string& script_filename,
        const std::string& working_directory,
        const UserFunction& execute_user_function,
        SubstitutionString& substitutions,
        bool verbose);
private:
    std::map<std::string, Macro> macros_;
};

}
