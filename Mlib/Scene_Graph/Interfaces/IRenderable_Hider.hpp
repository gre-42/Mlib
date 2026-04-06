#pragma once
#include <string>

namespace Mlib {

template <class T>
class VariableAndHash;

class IRenderableHider {
public:
    virtual void process_input() = 0;
    virtual bool is_visible(const VariableAndHash<std::string>& name) = 0;
};

}
