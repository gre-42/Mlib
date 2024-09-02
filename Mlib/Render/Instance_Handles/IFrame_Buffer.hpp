#pragma once
#include <Mlib/Source_Location.hpp>

namespace Mlib {

class IFrameBuffer {
public:
    virtual ~IFrameBuffer() = default;
    virtual bool is_configured() const = 0;
    virtual void bind(SourceLocation loc) = 0;
    virtual void unbind(SourceLocation loc) = 0;
};

}
