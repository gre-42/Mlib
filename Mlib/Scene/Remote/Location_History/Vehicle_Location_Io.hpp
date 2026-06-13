#pragma once
#include <Mlib/Os/Io/Binary_Bitwise_Words_Reader.hpp>
#include <Mlib/Os/Io/Binary_Bitwise_Words_Writer.hpp>
#include <Mlib/Remote/Incremental_Objects/Object_Lifetime_Status.hpp>
#include <optional>

namespace Mlib {

template <class TRemoteRigidBodyVehicleCache>
std::optional<typename TRemoteRigidBodyVehicleCache::Location> read_vehicle_location(
    TRemoteRigidBodyVehicleCache& cache,
    BinaryBitwiseWordsReader& reader,
    ObjectLifetimeStatus object_lifetime_status)
{
    using AbsoluteLocation8 = TRemoteRigidBodyVehicleCache::AbsoluteLocation8;
    using DeltaLocation = TRemoteRigidBodyVehicleCache::DeltaLocation;
    // Local
    {
        cache.local.remote_version = reader.read_binary<VersionType>("local.remote_version");
    }
    // Remote
    {
        auto base_version = reader.read_binary<VersionType>("base_version");
        auto new_version = reader.read_binary<VersionType>("new_version");
        if (base_version == 0) {
            auto initial_location = reader.deserialize<AbsoluteLocation8>("initial_location");
            if (object_lifetime_status == ObjectLifetimeStatus::DELETED) {
                return std::nullopt;
            }
            cache.remote.initialize8(new_version, initial_location);
        } else {
            auto delta = reader.deserialize<DeltaLocation>("delta");
            if (object_lifetime_status == ObjectLifetimeStatus::DELETED) {
                return std::nullopt;
            }
            return cache.remote.get_absolute_location(base_version, new_version, delta);
        }
    }
    return std::nullopt;
}

template <class TRemoteRigidBodyVehicleCache>
void write_vehicle_location(
    TRemoteRigidBodyVehicleCache& cache,
    BinaryBitwiseWordsWriter& writer,
    const typename TRemoteRigidBodyVehicleCache::Location& location)
{
    // Local
    {
        writer.write_binary(cache.remote.local_version, "remote.local_version");
    }
    // Remote
    {
        if (cache.local.remote_version == 0) {
            auto loc = cache.local.get_absolute8(location);
            writer.write_binary(VersionType(0), "base version = 0");
            writer.write_binary(cache.local.local_version, "local.local_version");
            writer.serialize(loc, "local.get_absolute8");
        } else {
            auto loc = cache.local.get_delta8(location);
            writer.write_binary(cache.local.remote_version, "base_version");
            writer.write_binary(cache.local.local_version, "new_version");
            writer.serialize(loc, "delta location");
        }
    }
}

}
