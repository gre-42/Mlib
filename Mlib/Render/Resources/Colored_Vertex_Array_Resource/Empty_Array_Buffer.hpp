#pragma once
#include <Mlib/Render/Instance_Handles/IArray_Buffer.hpp>

namespace Mlib {

class EmptyArrayBuffer: public IArrayBuffer {
    EmptyArrayBuffer(const EmptyArrayBuffer&) = delete;
    EmptyArrayBuffer& operator = (const EmptyArrayBuffer&) = delete;
public:
    EmptyArrayBuffer();
    ~EmptyArrayBuffer();

    virtual bool copy_in_progress() const override;
    virtual void wait() const override;
    virtual void update() override;
    virtual void bind() const override;

protected:
    virtual void set_type_erased(const char* begin, const char* end) override;
};

}
