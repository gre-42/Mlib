#pragma once
#include <Mlib/Math/Transformation/Quaternion.hpp>
#include <Mlib/Scene/Remote/Location_History/Avatar_Location_History_Entry.hpp>
#include <Mlib/Scene/Remote/Location_History/Vehicle_Location_History_Entry.hpp>
#include <Mlib/Scene_Config/Remote_Integers.hpp>
#include <optional>

namespace Mlib {

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
    std::vector<std::optional<TAbsoluteLocation8>> location_history =
        std::vector<std::optional<TAbsoluteLocation8>>(std::numeric_limits<DatagramIndexType>::max(), std::nullopt);
};

template <class TAbsoluteLocation8, class TDeltaLocation, class TLocation>
class RemoteRigidBodyVehicleRemoteHistory {
public:
    void initialize8(DatagramIndexType version, const TAbsoluteLocation8& l8);
    TLocation get_absolute_location(
        DatagramIndexType base_version, DatagramIndexType new_version, const TDeltaLocation& delta);
    bool has_local_version = false;
    std::vector<std::optional<TAbsoluteLocation8>> location_history =
        std::vector<std::optional<TAbsoluteLocation8>>(std::numeric_limits<DatagramIndexType>::max(), std::nullopt);
};

template <class TAbsoluteLocation8, class TDeltaLocation, class TLocation>
struct RemoteRigidBodyVehicleCache: public Object {
    using AbsoluteLocation8 = TAbsoluteLocation8;
    using DeltaLocation = TDeltaLocation;
    using Location = TLocation;
    RemoteRigidBodyVehicleLocalHistory<TAbsoluteLocation8, TDeltaLocation, TLocation> local;
    RemoteRigidBodyVehicleRemoteHistory<TAbsoluteLocation8, TDeltaLocation, TLocation> remote;
    std::optional<RemoteTimeCount> old_remote_time;
    Quaternion<SceneDir> old_remote_r{NAN, fixed_nans<SceneDir, 3>()};
    FixedArray<ScenePos, 3> old_remote_t{fixed_nans<ScenePos, 3>()};
};

using VehicleRemoteRigidBodyVehicleCache = RemoteRigidBodyVehicleCache<Vehicle::AbsoluteVehicleLocation8, Vehicle::DeltaVehicleLocation, Vehicle::VehicleLocation>;
using AvatarRemoteRigidBodyVehicleCache = RemoteRigidBodyVehicleCache<Avatar::AbsoluteVehicleLocation8, Avatar::DeltaVehicleLocation, Avatar::VehicleLocation>;

}
