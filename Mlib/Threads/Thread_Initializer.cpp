#include "Thread_Initializer.hpp"
#include <Mlib/Os/Os.hpp>
#include <Mlib/Threads/Thread_Affinity.hpp>

using namespace Mlib;

ThreadInitializer::ThreadInitializer(
    const std::string& name,
    ThreadAffinity affinity)
{
    set_thread_name(name);
    if (affinity == ThreadAffinity::DEDICATED) {
        rtg_.emplace();
    } else {
        pin_background_thread();
    }
}

ThreadInitializer::~ThreadInitializer() = default;
