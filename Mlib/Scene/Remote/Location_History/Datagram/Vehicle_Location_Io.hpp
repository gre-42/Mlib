#pragma once
#include <Mlib/Os/Io/Binary_Bitwise_Words_Reader.hpp>
#include <Mlib/Os/Io/Binary_Bitwise_Words_Writer.hpp>
#include <Mlib/Remote/Incremental_Objects/Incremental_Versions.hpp>
#include <Mlib/Remote/Incremental_Objects/Object_Lifetime_Status.hpp>
#include <Mlib/Scene_Config/Remote_Integers.hpp>
#include <optional>

namespace Mlib::Datagram {

template <class TRemoteRigidBodyVehicleCache>
void read_vehicle_location_and_forget(
    TRemoteRigidBodyVehicleCache& cache,
    BinaryBitwiseWordsReader& reader)
{
    using AbsoluteLocation8 = TRemoteRigidBodyVehicleCache::AbsoluteLocation8;
    using DeltaLocation = TRemoteRigidBodyVehicleCache::DeltaLocation;
    // Local
    {
        reader.read_bits<bool>(1, "local.has_remote_version");
    }
    // Remote
    {
        auto remote_has_new_version = reader.read_bits<bool>(1, "remote.has_new_version");
        if (!remote_has_new_version) {
            return;
        }
        auto remote_has_base_version = reader.read_bits<bool>(1, "remote.has_base_version");
        if (!remote_has_base_version) {
            reader.deserialize<AbsoluteLocation8>("initial_location");
        } else {
            reader.deserialize<DeltaLocation>("delta");
        }
    }
}

template <class TRemoteRigidBodyVehicleCache>
void write_dummy_vehicle_location(
    TRemoteRigidBodyVehicleCache& cache,
    BinaryBitwiseWordsWriter& writer,
    IoVerbosity verbosity)
{
    if (any(verbosity & IoVerbosity::METADATA)) {
        linfo() << "remote.has_local_version " << int(cache.remote.has_local_version);
    }
    writer.write_bits<uint8_t>(cache.remote.has_local_version, 1, "has_local_version");
    writer.write_bits<uint8_t>(false, 1, "local.local_version nonzero");
}

template <class TRemoteRigidBodyVehicleCache>
std::optional<typename TRemoteRigidBodyVehicleCache::Location> read_vehicle_location(
    TRemoteRigidBodyVehicleCache& cache,
    BinaryBitwiseWordsReader& reader,
    const IncrementalVersionsRead& versions)
{
    using AbsoluteLocation8 = TRemoteRigidBodyVehicleCache::AbsoluteLocation8;
    using DeltaLocation = TRemoteRigidBodyVehicleCache::DeltaLocation;
    // Local
    {
        cache.local.has_remote_version = reader.read_bits<bool>(1, "local.has_remote_version");
    }
    // Remote
    {
        auto has_new_version = reader.read_bits<bool>(1, "has_new_version");
        if (!has_new_version) {
            return std::nullopt;
        }
        auto has_base_version = reader.read_bits<bool>(1, "has_base_version");
        if (!has_base_version) {
            auto initial_location = reader.deserialize<AbsoluteLocation8>("initial_location");
            cache.remote.initialize8(versions.remote_new_version, initial_location);
        } else {
            auto delta = reader.deserialize<DeltaLocation>("delta");
            if (!cache.remote.has_local_version) {
                return std::nullopt;
            }
            return cache.remote.get_absolute_location(versions.remote_base_version, versions.remote_new_version, delta);
        }
    }
    return std::nullopt;
}

template <class TRemoteRigidBodyVehicleCache>
void write_vehicle_location(
    TRemoteRigidBodyVehicleCache& cache,
    BinaryBitwiseWordsWriter& writer,
    const typename TRemoteRigidBodyVehicleCache::Location& location,
    const IncrementalVersionsWrite& versions,
    IoVerbosity verbosity)
{
    // Local
    {
        if (any(verbosity & IoVerbosity::METADATA)) {
            linfo() << "remote.has_local_version " << int(cache.remote.has_local_version);
        }
        writer.write_bits<uint8_t>(cache.remote.has_local_version, 1, "remote.has_local_version");
    }
    // Remote
    {
        if (any(verbosity & IoVerbosity::METADATA)) {
            linfo() << "local.has_remote_version " << int(cache.local.has_remote_version);
        }
        if (cache.local.has_remote_version) {
            auto loc = cache.local.get_delta8(location, versions.local_base_version, versions.local_new_version);
            if (loc.has_value()) {
                writer.write_bits<uint8_t>(cache.local.has_local_version, 1, "local.has_new_version");
                writer.write_bits<uint8_t>(cache.local.has_remote_version, 1, "local.remote_version (=base_version)");
                writer.serialize(*loc, "delta location");
                return;
            }
            if (any(verbosity & IoVerbosity::METADATA)) {
                linfo() << "overflow";
            }
        }
        {
            auto loc = cache.local.get_absolute8(location, versions.local_new_version);
            writer.write_bits<uint8_t>(cache.local.has_local_version, 1, "local.local_version_nonzero");
            writer.write_bits<uint8_t>(false, 1, "base version nonzero");
            writer.serialize(loc, "local.get_absolute8");
        }
    }
}

}
