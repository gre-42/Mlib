#pragma once
#include <list>
#include <memory>
#include <set>

namespace Mlib {

class IPermanentContact;
class IContactInfo;
struct PhysicsEngineConfig;

struct PermanentContactComparator {
    using is_transparent = void;
    inline bool operator () (const std::unique_ptr<IPermanentContact>& left, const std::unique_ptr<IPermanentContact>& right) const {
        return left < right;
    }
    inline bool operator () (const std::unique_ptr<IPermanentContact>& left, const IPermanentContact* right) const {
        return left.get() < right;
    }
    inline bool operator () (const IPermanentContact* left, const std::unique_ptr<IPermanentContact>& right) const {
        return left < right.get();
    }
};

class PermanentContacts {
public:
    PermanentContacts();
    ~PermanentContacts();
    void insert(std::unique_ptr<IPermanentContact>&& permanent_contact);
    void remove(const IPermanentContact& permanent_contact);
    void extend_contact_infos(
        const PhysicsEngineConfig& cfg,
        std::list<std::unique_ptr<IContactInfo>>& contact_infos) const;
private:
    std::set<std::unique_ptr<IPermanentContact>, PermanentContactComparator> permanent_contacts_;
};

}
