#pragma once
#include <Mlib/Scene/Remote/Location_History/Avatar_Location_History_Entry.hpp>
#include <Mlib/Scene/Remote/Location_History/Vehicle_Location_History_Entry.hpp>
#include <Mlib/Scene_Config/Remote_Integers.hpp>

namespace Mlib::Datagram {

template <class TAbsoluteLocation8, class TDeltaLocation, class TLocation>
class RemoteRigidBodyVehicleLocalHistory {
public:
    TAbsoluteLocation8 get_absolute8(
        const TLocation& l,
        DatagramIndexType new_datagram_index);
    std::optional<TDeltaLocation> get_delta8(
        const TLocation& l,
        DatagramIndexType base_version,
        DatagramIndexType local_version);
    bool has_local_version = false;
    bool has_remote_version = false;
    std::vector<TAbsoluteLocation8> location_history =
        std::vector<TAbsoluteLocation8>(std::numeric_limits<DatagramIndexType>::max() + 1, TAbsoluteLocation8::nan());
private:
    DatagramIndexType size = 0;
};

template <class TAbsoluteLocation8, class TDeltaLocation, class TLocation>
class RemoteRigidBodyVehicleRemoteHistory {
public:
    void initialize8(DatagramIndexType version, const TAbsoluteLocation8& l8);
    TLocation get_absolute_location(
        DatagramIndexType base_version, DatagramIndexType new_version, const TDeltaLocation& delta);
    bool has_local_version = false;
    std::vector<TAbsoluteLocation8> location_history =
        std::vector<TAbsoluteLocation8>(std::numeric_limits<DatagramIndexType>::max(), TAbsoluteLocation8::nan());
private:
    DatagramIndexType size = 0;
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
