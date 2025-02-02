#include "Permanent_Contacts.hpp"
#include <Mlib/Os/Os.hpp>
#include <Mlib/Physics/Collision/Record/IPermanent_Contact.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

PermanentContacts::PermanentContacts() = default;

PermanentContacts::~PermanentContacts() = default;

void PermanentContacts::insert(std::unique_ptr<IPermanentContact>&& permanent_contact) {
    if (!permanent_contacts_.insert(std::move(permanent_contact)).second) {
        THROW_OR_ABORT("Could not insert permanent contact");
    }
}

void PermanentContacts::remove(const IPermanentContact& permanent_contact) {
    auto it = permanent_contacts_.find(&permanent_contact);
    if (it == permanent_contacts_.end()) {
        verbose_abort("Cannot find permanent contact to be deleted");
    }
    permanent_contacts_.erase(it);
}

void PermanentContacts::extend_contact_infos(
    const PhysicsEngineConfig& cfg,
    std::list<std::unique_ptr<IContactInfo>>& contact_infos) const
{
    for (const auto& pc : permanent_contacts_) {
        pc->extend_contact_infos(cfg, contact_infos);
    }
}
