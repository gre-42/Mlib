#pragma once
#include <Mlib/Os/Io/Binary_Bitwise_Words_Reader.hpp>
#include <Mlib/Os/Io/Binary_Bitwise_Words_Writer.hpp>
#include <Mlib/Remote/Incremental_Objects/Object_Lifetime_Status.hpp>
#include <optional>

namespace Mlib {

template <class TRemoteRigidBodyVehicleCache>
void read_vehicle_location_and_forget(
    TRemoteRigidBodyVehicleCache& cache,
    BinaryBitwiseWordsReader& reader)
{
    using AbsoluteLocation8 = TRemoteRigidBodyVehicleCache::AbsoluteLocation8;
    using DeltaLocation = TRemoteRigidBodyVehicleCache::DeltaLocation;
    // Local
    {
        reader.read_binary<VersionType>("local.remote_version");
    }
    // Remote
    {
        auto new_version = reader.read_binary<VersionType>("new_version");
        if (new_version == 0) {
            return;
        }
        auto base_version = reader.read_binary<VersionType>("base_version");
        if (base_version == 0) {
            reader.deserialize<AbsoluteLocation8>("initial_location");
        } else {
            reader.deserialize<DeltaLocation>("delta");
        }
    }
}

template <class TRemoteRigidBodyVehicleCache>
void write_dummy_vehicle_location(
    TRemoteRigidBodyVehicleCache& cache,
    BinaryBitwiseWordsWriter& writer)
{
    writer.write_binary(cache.remote.local_version, "remote.local_version");
    writer.write_binary(VersionType(0), "local.local_version = 0");
}

template <class TRemoteRigidBodyVehicleCache>
std::optional<typename TRemoteRigidBodyVehicleCache::Location> read_vehicle_location(
    TRemoteRigidBodyVehicleCache& cache,
    BinaryBitwiseWordsReader& reader)
{
    using AbsoluteLocation8 = TRemoteRigidBodyVehicleCache::AbsoluteLocation8;
    using DeltaLocation = TRemoteRigidBodyVehicleCache::DeltaLocation;
    // Local
    {
        cache.local.remote_version = reader.read_binary<VersionType>("local.remote_version");
    }
    // Remote
    {
        auto new_version = reader.read_binary<VersionType>("new_version");
        if (new_version == 0) {
            return std::nullopt;
        }
        auto base_version = reader.read_binary<VersionType>("base_version");
        if (base_version == 0) {
            auto initial_location = reader.deserialize<AbsoluteLocation8>("initial_location");
            cache.remote.initialize8(new_version, initial_location);
        } else {
            auto delta = reader.deserialize<DeltaLocation>("delta");
            if (cache.remote.local_version == 0) {
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
        if (cache.local.remote_version != 0) {
            auto loc = cache.local.get_delta8(location);
            if (loc.has_value()) {
                writer.write_binary(cache.local.local_version, "new_version");
                writer.write_binary(cache.local.remote_version, "base_version");
                writer.serialize(*loc, "delta location");
                return;
            } else {
                cache.local.remote_version = 0;
            }
        }
        {
            auto loc = cache.local.get_absolute8(location);
            writer.write_binary(cache.local.local_version, "local.local_version");
            writer.write_binary(VersionType(0), "base version = 0");
            writer.serialize(loc, "local.get_absolute8");
        }
    }
}

}
