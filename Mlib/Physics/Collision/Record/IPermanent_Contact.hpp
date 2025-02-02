#pragma once
#include <list>
#include <memory>

namespace Mlib {

class IContactInfo;
struct PhysicsEngineConfig;

class IPermanentContact {
public:
    virtual ~IPermanentContact() = default;
    virtual void extend_contact_infos(
        const PhysicsEngineConfig& cfg,
        std::list<std::unique_ptr<IContactInfo>>& contact_infos) = 0;
};

}
