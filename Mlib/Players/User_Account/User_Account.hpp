#pragma once
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <cstdint>
#include <string>

namespace Mlib {

class MacroLineExecutor;

class UserAccount {
public:
    UserAccount(
        const MacroLineExecutor& mle,
        std::string name_key);
    std::string name() const;
private:
    MacroLineExecutor mle_;
    std::string name_key_;
};

}
