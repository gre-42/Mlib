#pragma once
#include <Mlib/Macro_Executor/Notifying_Json_Macro_Arguments.hpp>

namespace Mlib {

class MacroLineExecutor;
class UiFocuses;

class FocusFinalizer {
    FocusFinalizer(const FocusFinalizer&) = delete;
    FocusFinalizer& operator = (const FocusFinalizer&) = delete;
public:
    FocusFinalizer(
        UiFocuses& ui_focuses,
        MacroLineExecutor& mle);
    ~FocusFinalizer();
private:
    JsonMacroArgumentsObserverToken ot_;
};

}
