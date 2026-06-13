#pragma once
#include <Mlib/Scene/Remote/Location_History/Avatar_Location_History_Entry.hpp>
#include <Mlib/Scene/Remote/Location_History/Vehicle_Location_History_Entry.hpp>

namespace Mlib {

using VersionType = uint8_t;

template <class TAbsoluteLocation8, class TDeltaLocation, class TLocation>
class RemoteRigidBodyVehicleLocalHistory {
public:
    TAbsoluteLocation8 get_absolute8(const TLocation& l);
    TDeltaLocation get_delta8(const TLocation& l);
    VersionType local_version = 0;
    VersionType remote_version = 0;
    std::vector<TAbsoluteLocation8> location_history =
        std::vector<TAbsoluteLocation8>(std::numeric_limits<VersionType>::max(), TAbsoluteLocation8::nan());
private:
    void increase_local_version();
};

template <class TAbsoluteLocation8, class TDeltaLocation, class TLocation>
class RemoteRigidBodyVehicleRemoteHistory {
public:
    void initialize8(VersionType version, const TAbsoluteLocation8& l8);
    TLocation get_absolute_location(
        VersionType base_version, VersionType new_version, const TDeltaLocation& delta);
    VersionType local_version = 0;
    std::vector<TAbsoluteLocation8> location_history =
        std::vector<TAbsoluteLocation8>(std::numeric_limits<VersionType>::max(), TAbsoluteLocation8::nan());
};

template <class TAbsoluteLocation8, class TDeltaLocation, class TLocation>
struct RemoteRigidBodyVehicleCache: public Object {
    using AbsoluteLocation8 = TAbsoluteLocation8;
    using DeltaLocation = TDeltaLocation;
    using Location = TLocation;
    RemoteRigidBodyVehicleLocalHistory<TAbsoluteLocation8, TDeltaLocation, TLocation> local;
    RemoteRigidBodyVehicleRemoteHistory<TAbsoluteLocation8, TDeltaLocation, TLocation> remote;
};

using VehicleRemoteRigidBodyVehicleCache = RemoteRigidBodyVehicleCache<Vehicle::AbsoluteVehicleLocation8, Vehicle::DeltaVehicleLocation, Vehicle::VehicleLocation>;
using AvatarRemoteRigidBodyVehicleCache = RemoteRigidBodyVehicleCache<Avatar::AbsoluteVehicleLocation8, Avatar::DeltaVehicleLocation, Avatar::VehicleLocation>;

}
