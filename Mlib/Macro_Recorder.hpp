#pragma once
#include <functional>
#include <list>
#include <map>
#include <string>

namespace Mlib {

class RegexSubstitutionCache;
class MacroLineExecutor;

struct Macro {
    std::string filename;
    std::list<std::string> lines;
};

class MacroRecorder {
    friend MacroLineExecutor;
public:
    void operator () (const MacroLineExecutor& macro_line_executor, const RegexSubstitutionCache& rsc);
private:
    std::map<std::string, Macro> macros_;
    std::map<std::string, std::string> aliases_;
};

}
