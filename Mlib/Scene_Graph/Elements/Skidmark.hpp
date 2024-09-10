#pragma once
#include <Mlib/Variable_And_Hash.hpp>
#include <string>

namespace Mlib {

struct Skidmark {
    VariableAndHash<std::string> resource_name;
};

}
