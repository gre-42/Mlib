#pragma once
#include <string>

namespace Mlib {

class JsonView;

template <class T>
T eval(
    const std::string& expression,
    const JsonView& globals,
    const JsonView& locals);

template <class T>
T eval(const std::string& expression, const JsonView& variables);

}
