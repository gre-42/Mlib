#pragma once

namespace Mlib {

class NodeModifier {
public:
    virtual ~NodeModifier() = default;
    virtual void modify_node() = 0;
};

}
