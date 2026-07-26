#include "Remote_Privileges.hpp"

using namespace Mlib;

RemotePrivileges::RemotePrivileges(
    RemoteSiteId local_site,
    RemoteSiteId update_sender,
    RemoteSiteId object_owner,
    RemoteSiteId object_manager)
{
    is_manager_local = (object_manager == local_site);
    is_manager_sender = (object_manager == update_sender);
    is_owner_local = (object_owner == local_site);
    is_owner_sender = (object_owner == update_sender);
};

PositionPrivileges RemotePrivileges::position(PositionFlags flags) {
    PositionPrivileges result;
    bool accept_coordinates = [&](){
        if (any(flags & PositionFlags::POSITION_IS_INCOMPLETE) ||
            any(flags & PositionFlags::IS_DEACTIVATED_AVATAR))
        {
            return false;
        }
        if (is_manager_local) {
            return !any(flags & PositionFlags::POSITION_CONTAINS_JUMP) && is_owner_sender;
        } else {
            return true;
        }
    }();
    result.update_physics = [&](){
        if (!accept_coordinates) {
            return false;
        }
        if (is_manager_local) {
            return is_owner_sender;
        } else {
            return is_manager_sender; 
        }
    }();
    result.invalidate_transformation_history = [&](){
        if (!accept_coordinates) {
            return false;
        }
        return any(flags & PositionFlags::POSITION_CONTAINS_JUMP) ||
               (flags == PositionFlags::IS_REMOTELY_ACTIVATED_AVATAR);
    }();
    result.update_position = [&](){
        if (!accept_coordinates) {
            return false;
        }
        if (result.invalidate_transformation_history) {
            return true;
        }
        return !is_owner_local;
    }();
    return result;
}
