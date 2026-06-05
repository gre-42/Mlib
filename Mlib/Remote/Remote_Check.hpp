#pragma once

namespace Mlib {

enum class RemoteEndCheckOption {
    ENABLED,
    DISABLED
};

void set_remote_end_check(RemoteEndCheckOption option);
bool remote_end_check_enabled();

}
