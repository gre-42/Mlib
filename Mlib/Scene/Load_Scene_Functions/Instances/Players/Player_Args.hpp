#pragma once
#include <Mlib/Misc/Argument_List.hpp>

namespace Mlib {

namespace PlayerArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(team);
DECLARE_ARGUMENT(full_user_name);
DECLARE_ARGUMENT(user_account_key);
DECLARE_ARGUMENT(game_mode);
DECLARE_ARGUMENT(player_role);
DECLARE_ARGUMENT(unstuck_mode);
DECLARE_ARGUMENT(behavior);
DECLARE_ARGUMENT(driving_direction);
}

}
