#pragma once
#include <Mlib/OpenGL/IContext.hpp>

namespace Mlib {

class AContext: public IContext {
public:
    AContext();
    ~AContext();
    bool is_initialized() const override;
};

}
