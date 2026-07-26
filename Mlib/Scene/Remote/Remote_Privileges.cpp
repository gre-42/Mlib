#include "Remote_Privileges.hpp"
#include <Mlib/Remote/Incremental_Objects/Proxy_Tasks.hpp>

using namespace Mlib;

RemotePrivileges::RemotePrivileges(
    ProxyTasks local_proxy_tasks,
    RemoteSiteId local_site,
    RemoteSiteId update_sender,
    RemoteSiteId object_owner)
{
    is_server_local = any(local_proxy_tasks & ProxyTasks::SEND_OWNERSHIP);
    is_owner_local = (object_owner == local_site);
    is_owner_sender = (object_owner == update_sender);
};

PositionPrivileges RemotePrivileges::position(PositionFlags flags) {
    PositionPrivileges result;
    // True if the coordinates are supposed
    // to be stored in the location cache.
    bool accept_coordinates = [&](){
        if (any(flags & PositionFlags::POSITION_IS_INCOMPLETE) ||
            any(flags & PositionFlags::IS_DEACTIVATED_AVATAR))
        {
            return false;
        }
        if (is_server_local) {
            if (!is_owner_sender) {
                return false;
            }
            if (any(flags & PositionFlags::WAITING_FOR_POSITION)) {
                return true;
            }
            return !any(flags & PositionFlags::POSITION_CONTAINS_JUMP);
        } else {
            return true;
        }
    }();
    // True if the coordinates are to be
    // applied to the physics simulation.
    result.update_physics = [&](){
        if (!accept_coordinates) {
            return false;
        }
        if (is_server_local) {
            return is_owner_sender;
        } else {
            return true; 
        }
    }();
    result.invalidate_transformation_history = [&](){
        if (!accept_coordinates) {
            return false;
        }
        if (is_server_local) {
            return any(flags & PositionFlags::WAITING_FOR_POSITION);
        }
        return any(flags & PositionFlags::POSITION_CONTAINS_JUMP) ||
               any(flags & PositionFlags::IS_REMOTELY_ACTIVATED_AVATAR) ||
               any(flags & PositionFlags::WAITING_FOR_POSITION);
    }();
    result.update_position = [&](){
        if (!accept_coordinates) {
            return false;
        }
        if (result.invalidate_transformation_history) {
            return true;
        }
        return !is_owner_local ||
               any(flags & PositionFlags::WAITING_FOR_POSITION) ||
               any(flags & PositionFlags::WAITING_FOR_VELOCITY);
    }();
    return result;
}
