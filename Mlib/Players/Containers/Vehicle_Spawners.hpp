#pragma once
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
    VehicleSpawner& get(const std::string& name);
    void set(const std::string& name, std::unique_ptr<VehicleSpawner>&& spawner);
    inline std::map<std::string, std::unique_ptr<VehicleSpawner>>& spawners() {
        return spawners_;
    }
    void advance_time(float dt);
    size_t nactive() const;
private:
    std::map<std::string, std::unique_ptr<VehicleSpawner>> spawners_;
};

}
