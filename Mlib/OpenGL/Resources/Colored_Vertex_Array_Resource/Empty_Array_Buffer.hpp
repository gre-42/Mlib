#pragma once
#include <Mlib/OpenGL/Instance_Handles/IArray_Buffer.hpp>

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
    virtual bool is_initialized() const override;
    virtual std::shared_ptr<IArrayBuffer> fork() override;

protected:
    virtual void init_type_erased(
        const std::byte* begin,
        const std::byte* end) override;
    virtual void substitute_type_erased(
        const std::byte* begin,
        const std::byte* end) override;
};

}
