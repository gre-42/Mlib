#pragma once
#include <Mlib/Map/String_With_Hash_Unordered_Map.hpp>
#include <list>
#include <map>
#include <memory>
#include <string>

namespace Mlib {

class VehicleSpawner;

class VehicleSpawners {
public:
    VehicleSpawners();
    ~VehicleSpawners();
    VehicleSpawner& get(const VariableAndHash<std::string>& name);
    void set(const VariableAndHash<std::string>& name, std::unique_ptr<VehicleSpawner>&& spawner);
    inline StringWithHashUnorderedMap<std::unique_ptr<VehicleSpawner>>& spawners() {
        return spawners_;
    }
    void advance_time(float dt);
    size_t nactive() const;
    void print_status() const;
private:
    StringWithHashUnorderedMap<std::unique_ptr<VehicleSpawner>> spawners_;
};

}
