#include "Remote_Privileges.hpp"

using namespace Mlib;

RemotePrivileges::RemotePrivileges(
    RemoteSiteId local_site,
    RemoteSiteId update_sender,
    RemoteSiteId object_owner,
    RemoteSiteId object_manager)
{
    is_manager_local = (object_manager == local_site);
    is_owner_local = (object_owner == local_site);
    is_owner_sender = (object_owner == update_sender);
};

PositionPrivileges RemotePrivileges::position(PositionFlags flags) {
    PositionPrivileges result;
    result.invalidate_transformation_history = [&](){
        if (is_manager_local) {
            return false;
        }
        if (any(flags & PositionFlags::WAITING_FOR_INITIAL_POSITION)) {
            return false;
        }
        return any(flags & PositionFlags::POSITION_CONTAINS_JUMP) ||
               (flags == PositionFlags::IS_REMOTELY_ACTIVATED_AVATAR);
    }();
    result.update_position = [&](){
        if (any(flags & PositionFlags::WAITING_FOR_INITIAL_POSITION)) {
            return false;
        }
        if (is_manager_local) {
            return !any(flags & PositionFlags::POSITION_CONTAINS_JUMP) && is_owner_sender;
        } else {
            return result.invalidate_transformation_history || !is_owner_local;
        }
    }();
    return result;
}
