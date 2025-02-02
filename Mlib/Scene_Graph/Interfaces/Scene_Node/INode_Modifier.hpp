#pragma once

namespace Mlib {

class INodeModifier {
public:
    virtual ~INodeModifier() = default;
    virtual void modify_node() = 0;
};

}
