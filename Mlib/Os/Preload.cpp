#include "Preload.hpp"
#include <Mlib/Os/Threads/Fast_Mutex.hpp>

using namespace Mlib;

static NotPreloadedBehavior not_preloaded_behavior = NotPreloadedBehavior::SILENT;
static FastMutex not_preloaded_behavior_mutex;

void Mlib::set_not_preloaded_behavior(NotPreloadedBehavior value) {
    std::scoped_lock lock{not_preloaded_behavior_mutex};
    not_preloaded_behavior = value;
}

NotPreloadedBehavior Mlib::get_not_preloaded_behavior() {
    std::scoped_lock lock{not_preloaded_behavior_mutex};
    return not_preloaded_behavior;
}
