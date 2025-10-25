#pragma once
#include <string_view>

namespace Mlib {

enum class RemoteRole {
    NONE,
    SERVER,
    CLIENT
};

RemoteRole remote_role_from_string(const std::string_view& s);
std::string_view remote_role_to_string(RemoteRole r);

}
