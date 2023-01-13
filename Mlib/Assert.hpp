#pragma once
#include <Mlib/Throw_Or_Abort.hpp>

#define assert_true(x) if (!(x)) THROW_OR_ABORT(std::string("Assertion failed: ") + #x)
