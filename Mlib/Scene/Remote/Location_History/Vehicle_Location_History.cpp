#include "Vehicle_Location_History.hpp"
#include <Mlib/Scene/Remote/Location_History/Vehicle_Location_History_Entry.hpp>

using namespace Mlib;

// =========
// | Local |
// =========
template <class TAbsoluteLocation8, class TDeltaLocation, class TLocation>
TAbsoluteLocation8 RemoteRigidBodyVehicleLocalHistory<TAbsoluteLocation8, TDeltaLocation, TLocation>::get_absolute8(
    const TLocation& l)
{
    increase_local_version();
    auto& new_ = location_history[local_version - 1];
    new_ = l.fixed_point().downsample();
    return new_;
}

template <class TAbsoluteLocation8, class TDeltaLocation, class TLocation>
TDeltaLocation RemoteRigidBodyVehicleLocalHistory<TAbsoluteLocation8, TDeltaLocation, TLocation>::get_delta8(
    const TLocation& l)
{
    if (remote_version == 0) {
        throw std::runtime_error("Incremental location requires remote version > 0");
    }
    increase_local_version();
    const auto& base = location_history[remote_version - 1];
    auto& new_ = location_history[local_version - 1];
    auto f = l.fixed_point();
    new_ = f.downsample();
    return f - base;
}

template <class TAbsoluteLocation8, class TDeltaLocation, class TLocation>
void RemoteRigidBodyVehicleLocalHistory<TAbsoluteLocation8, TDeltaLocation, TLocation>::increase_local_version() {
    local_version = std::max<VersionType>(1, local_version + 1);
}

// ==========
// | Remote |
// ==========
template <class TAbsoluteLocation8, class TDeltaLocation, class TLocation>
void RemoteRigidBodyVehicleRemoteHistory<TAbsoluteLocation8, TDeltaLocation, TLocation>::initialize8(
    VersionType version, const TAbsoluteLocation8& l8)
{
    if (version == 0) {
        throw std::runtime_error("Absolute location requires version > 0");
    }
    location_history[version - 1] = l8;
    local_version = version;
}

template <class TAbsoluteLocation8, class TDeltaLocation, class TLocation>
TLocation RemoteRigidBodyVehicleRemoteHistory<TAbsoluteLocation8, TDeltaLocation, TLocation>::get_absolute_location(
    VersionType base_version, VersionType new_version, const TDeltaLocation& delta)
{
    if (local_version == 0) {
        throw std::runtime_error(
            (std::stringstream() << "Incremental location requires local version > 0. " <<
            "Local: " << (local_version + 0) <<
            ", base: " << (base_version + 0) <<
            ", new: " << (new_version + 0)).str());
    }
    if (new_version == 0) {
        throw std::runtime_error(
            (std::stringstream() << "Incremental location requires new version > 0. " <<
            "Local: " << (local_version + 0) <<
            ", base: " << (base_version + 0) <<
            ", new: " << (new_version + 0)).str());
    }
    const auto& base = location_history[base_version - 1];
    auto& new_ = location_history[new_version - 1];
    auto f = base + delta;
    new_ = f.downsample();
    local_version = new_version;
    return f.floating_point();
}

template class Mlib::RemoteRigidBodyVehicleLocalHistory<Vehicle::AbsoluteVehicleLocation8, Vehicle::DeltaVehicleLocation, Vehicle::VehicleLocation>;
template class Mlib::RemoteRigidBodyVehicleRemoteHistory<Vehicle::AbsoluteVehicleLocation8, Vehicle::DeltaVehicleLocation, Vehicle::VehicleLocation>;
template struct Mlib::RemoteRigidBodyVehicleCache<Vehicle::AbsoluteVehicleLocation8, Vehicle::DeltaVehicleLocation, Vehicle::VehicleLocation>;

template class Mlib::RemoteRigidBodyVehicleLocalHistory<Avatar::AbsoluteVehicleLocation8, Avatar::DeltaVehicleLocation, Avatar::VehicleLocation>;
template class Mlib::RemoteRigidBodyVehicleRemoteHistory<Avatar::AbsoluteVehicleLocation8, Avatar::DeltaVehicleLocation, Avatar::VehicleLocation>;
template struct Mlib::RemoteRigidBodyVehicleCache<Avatar::AbsoluteVehicleLocation8, Avatar::DeltaVehicleLocation, Avatar::VehicleLocation>;
