#include "Remote_Check.hpp"

using namespace Mlib;

static RemoteEndCheckOption global_option = RemoteEndCheckOption::DISABLED;

void Mlib::set_remote_end_check(RemoteEndCheckOption option) {
    global_option = option;
}

bool Mlib::remote_end_check_enabled() {
    return global_option == RemoteEndCheckOption::ENABLED;
}
