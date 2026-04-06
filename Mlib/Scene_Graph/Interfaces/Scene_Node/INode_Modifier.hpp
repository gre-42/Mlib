#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>

namespace Mlib {

class INodeModifier: public virtual DanglingBaseClass {
public:
    virtual ~INodeModifier() = default;
    virtual void modify_node() = 0;
};

}
