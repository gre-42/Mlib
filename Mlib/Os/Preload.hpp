#pragma once

namespace Mlib {

enum class NotPreloadedBehavior {
    WARN,
    SILENT
};

void set_not_preloaded_behavior(NotPreloadedBehavior value);
NotPreloadedBehavior get_not_preloaded_behavior();

}
