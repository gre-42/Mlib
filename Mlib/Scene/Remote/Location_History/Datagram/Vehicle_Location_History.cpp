#include "Vehicle_Location_History.hpp"
#include <Mlib/Scene/Remote/Location_History/Vehicle_Location_History_Entry.hpp>

using namespace Mlib;
using namespace Mlib::Datagram;

// =========
// | Local |
// =========
template <class TAbsoluteLocation8, class TDeltaLocation, class TLocation>
TAbsoluteLocation8 RemoteRigidBodyVehicleLocalHistory<TAbsoluteLocation8, TDeltaLocation, TLocation>::get_absolute8(
    const TLocation& l,
    DatagramIndexType new_datagram_index)
{
    if (new_datagram_index == 0) {
        throw std::runtime_error("New datagram index is zero");
    }
    has_local_version = true;
    auto& new_ = location_history.at(new_datagram_index - 1);
    new_ = l.fixed_point().downsample();
    size = std::max(size, new_datagram_index);
    return new_;
}

template <class TAbsoluteLocation8, class TDeltaLocation, class TLocation>
std::optional<TDeltaLocation> RemoteRigidBodyVehicleLocalHistory<TAbsoluteLocation8, TDeltaLocation, TLocation>::get_delta8(
    const TLocation& l,
    DatagramIndexType base_version,
    DatagramIndexType local_version)
{
    if (base_version == 0) {
        throw std::runtime_error("Incremental location requires base version > 0");
    }
    if (!(base_version - 1 < size)) {
        throw std::runtime_error((std::stringstream() <<
            "Base version exceeds history size (0). Base version " << (base_version + 0) <<
            ", size " << (size + 0)).str());
    }
    if (local_version == 0) {
        throw std::runtime_error("Incremental location requires local version > 0");
    }
    const auto& base = location_history.at(base_version - 1);
    auto f = l.fixed_point();
    auto c = IncrementalConfig::NONE;
    auto diff = minus_position(f, base, c);
    if (any(c & IncrementalConfig::OVERFLOW)) {
        has_local_version = false;
        return std::nullopt;
    }
    has_local_version = true;
    auto& new_ = location_history.at(local_version - 1);
    new_ = f.downsample();
    size = std::max(size, local_version);
    return diff;
}

// ==========
// | Remote |
// ==========
template <class TAbsoluteLocation8, class TDeltaLocation, class TLocation>
void RemoteRigidBodyVehicleRemoteHistory<TAbsoluteLocation8, TDeltaLocation, TLocation>::initialize8(
    DatagramIndexType version, const TAbsoluteLocation8& l8)
{
    if (version == 0) {
        throw std::runtime_error("Absolute location requires version > 0");
    }
    location_history.at(version - 1) = l8;
    size = std::max(size, version);
    has_local_version = true;
}

template <class TAbsoluteLocation8, class TDeltaLocation, class TLocation>
TLocation RemoteRigidBodyVehicleRemoteHistory<TAbsoluteLocation8, TDeltaLocation, TLocation>::get_absolute_location(
    DatagramIndexType base_version, DatagramIndexType new_version, const TDeltaLocation& delta)
{
    if (!has_local_version || (base_version == 0) || (new_version == 0)) {
        throw std::runtime_error(
            (std::stringstream() << "Incremental location requires versions > 0. " <<
            "Has local version: " << int(has_local_version) <<
            ", base: " << (base_version + 0) <<
            ", new: " << (new_version + 0)).str());
    }
    if (!(base_version - 1 < size)) {
        throw std::runtime_error((std::stringstream() <<
            "Base version exceeds history size (1). Base version " << (base_version + 0) <<
            ", size " << (size + 0)).str());
    }
    const auto& base = location_history.at(base_version - 1);
    auto& new_ = location_history.at(new_version - 1);
    auto f = base + delta;
    new_ = f.downsample();
    size = std::max(size, new_version);
    return f.floating_point();
}

template class Datagram::RemoteRigidBodyVehicleLocalHistory<Vehicle::AbsoluteVehicleLocation8, Vehicle::DeltaVehicleLocation, Vehicle::VehicleLocation>;
template class Datagram::RemoteRigidBodyVehicleRemoteHistory<Vehicle::AbsoluteVehicleLocation8, Vehicle::DeltaVehicleLocation, Vehicle::VehicleLocation>;
template struct Datagram::RemoteRigidBodyVehicleCache<Vehicle::AbsoluteVehicleLocation8, Vehicle::DeltaVehicleLocation, Vehicle::VehicleLocation>;

template class Datagram::RemoteRigidBodyVehicleLocalHistory<Avatar::AbsoluteVehicleLocation8, Avatar::DeltaVehicleLocation, Avatar::VehicleLocation>;
template class Datagram::RemoteRigidBodyVehicleRemoteHistory<Avatar::AbsoluteVehicleLocation8, Avatar::DeltaVehicleLocation, Avatar::VehicleLocation>;
template struct Datagram::RemoteRigidBodyVehicleCache<Avatar::AbsoluteVehicleLocation8, Avatar::DeltaVehicleLocation, Avatar::VehicleLocation>;
