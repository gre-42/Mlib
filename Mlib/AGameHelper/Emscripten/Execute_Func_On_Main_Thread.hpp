#pragma once
#include <functional>

namespace Mlib {

void execute_func_on_main_thread(const std::function<void()>& func);

}
