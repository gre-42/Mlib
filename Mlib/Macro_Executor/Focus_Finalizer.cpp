#include "Focus_Finalizer.hpp"
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>

using namespace Mlib;

FocusFinalizer::FocusFinalizer(
    UiFocuses& ui_focuses,
    MacroLineExecutor& mle)
    : ot_{ mle.add_finalizer([&](){
        ui_focuses.pop_invalid_focuses(mle);
    }) }
{}

FocusFinalizer::~FocusFinalizer() = default;
