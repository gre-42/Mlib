#pragma once
#include <Mlib/Threads/Recursive_Shared_Mutex.hpp>
#include <functional>
#include <map>
#include <memory>
#include <string>

namespace Mlib {

class IParticlesResource;

class ParticlesResources {
public:
    ParticlesResources();
    ~ParticlesResources();
    void insert(
        const std::string& name,
        const std::function<std::shared_ptr<IParticlesResource>()>& create_resource);
    void insert(
        const std::string& name,
        const std::shared_ptr<IParticlesResource>& resource);
    std::shared_ptr<IParticlesResource> get(const std::string& name) const;
private:
    std::map<std::string, std::function<std::shared_ptr<IParticlesResource>()>> resource_creators_;
    mutable std::map<std::string, std::shared_ptr<IParticlesResource>> resources_;
    mutable RecursiveSharedMutex mutex_;
};

}
