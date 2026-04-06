#pragma once
#include <Mlib/Macro_Executor/Boolean_Expression.hpp>
#include <string>

namespace Mlib {

struct KeyDescription {
    BooleanExpression required;
    std::string unique;
    std::string id;
    std::string section;
    std::string title;
};

}
