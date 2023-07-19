#include "Render_Allocator.hpp"
#include <Mlib/Memory/Recursive_Deletion.hpp>
#include <list>
#include <mutex>

using namespace Mlib;

static std::list<std::function<void()>> funcs;
static std::mutex mutex;

void Mlib::execute_render_allocators() {
    std::scoped_lock lock{mutex};
    clear_list_recursively(funcs, [](const auto& f){f();});
}

void Mlib::append_render_allocator(const std::function<void()>& func) {
    std::scoped_lock lock{mutex};
    funcs.push_back(func);
}
