#pragma once
#include <functional>

namespace Mlib {

void execute_render_allocators();
void append_render_allocator(const std::function<void()>& func);

}
