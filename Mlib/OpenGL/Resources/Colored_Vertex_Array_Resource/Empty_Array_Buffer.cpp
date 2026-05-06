#include "Empty_Array_Buffer.hpp"
#include <stdexcept>

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
    throw std::runtime_error("EmptyArrayBuffer::bind called");
}

bool EmptyArrayBuffer::is_initialized() const {
    throw std::runtime_error("EmptyArrayBuffer::is_initialized called");
}

std::shared_ptr<IArrayBuffer> EmptyArrayBuffer::fork() {
    throw std::runtime_error("EmptyArrayBuffer::fork called");
}

void EmptyArrayBuffer::init_type_erased(
    const std::byte* begin,
    const std::byte* end)
{
    throw std::runtime_error("EmptyArrayBuffer::init_type_erased called");
}

void EmptyArrayBuffer::substitute_type_erased(
    const std::byte* begin,
    const std::byte* end)
{
    throw std::runtime_error("EmptyArrayBuffer::substitute_type_erased called");
}
