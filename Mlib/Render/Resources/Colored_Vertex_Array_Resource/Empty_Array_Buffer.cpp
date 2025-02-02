#include "Empty_Array_Buffer.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

EmptyArrayBuffer::EmptyArrayBuffer() = default;

EmptyArrayBuffer::~EmptyArrayBuffer() = default;

bool EmptyArrayBuffer::copy_in_progress() const {
    return false;
}

void EmptyArrayBuffer::wait() const {
    // Do nothing
}

void EmptyArrayBuffer::update() {
    // Do nothing
}

void EmptyArrayBuffer::bind() const {
    THROW_OR_ABORT("EmptyArrayBuffer::bind called");
}

bool EmptyArrayBuffer::is_awaited() const {
    THROW_OR_ABORT("EmptyArrayBuffer::is_awaited called");
}

std::shared_ptr<IArrayBuffer> EmptyArrayBuffer::fork() {
    THROW_OR_ABORT("EmptyArrayBuffer::fork called");
}

void EmptyArrayBuffer::set_type_erased(
    const char* begin,
    const char* end,
    TaskLocation task_location)
{
    THROW_OR_ABORT("EmptyArrayBuffer::set_type_erased called");
}
