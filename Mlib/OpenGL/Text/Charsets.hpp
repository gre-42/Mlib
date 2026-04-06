#pragma once
#include <Mlib/Hashing/Variable_And_Hash.hpp>
#include <cstdint>
#include <string>

namespace Mlib {

static const VariableAndHash<std::string> ascii{ "ASCII" };
std::u32string ascii_chars();

}
