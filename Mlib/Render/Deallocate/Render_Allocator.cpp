#include "Render_Allocator.hpp"
#include <Mlib/Memory/Recursive_Deletion.hpp>
#include <Mlib/Threads/Atomic_Mutex.hpp>
#include <list>
#include <mutex>

using namespace Mlib;

static std::list<std::function<void()>> funcs;
static AtomicMutex mutex;

void Mlib::execute_render_allocators() {
    std::unique_lock lock{ mutex };
    clear_list_recursively_with_lock(
        funcs,
        lock,
        [](const auto& f){f();});
}

void Mlib::append_render_allocator(std::function<void()> func) {
    std::scoped_lock lock{ mutex };
    funcs.push_back(std::move(func));
}

void Mlib::discard_render_allocators() {
    std::unique_lock lock{ mutex };
    funcs.clear();
}
