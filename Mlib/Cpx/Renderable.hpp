#pragma once
#include <ostream>

namespace Mlib { namespace cpx {

class Renderable {
public:
    virtual ~Renderable() = default;
    virtual void render(std::ostream& ostream) const = 0;
};

std::ostream& operator << (std::ostream& ostream, const Renderable& renderable) {
    renderable.render(ostream);
    return ostream;
}

}}
