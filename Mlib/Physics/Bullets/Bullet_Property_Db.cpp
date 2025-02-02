#include "Bullet_Property_Db.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

BulletPropertyDb::BulletPropertyDb() = default;

BulletPropertyDb::~BulletPropertyDb() = default;

void BulletPropertyDb::add(std::string name, BulletProperties&& props) {
    if (!properties_.try_emplace(std::move(name), std::move(props)).second) {
        THROW_OR_ABORT("Bullet properties with name \"" + name + "\" already exist");
    }
}

const BulletProperties& BulletPropertyDb::get(const std::string& name) {
    auto it = properties_.find(name);
    if (it == properties_.end()) {
        THROW_OR_ABORT("Could not find bullet properties with name \"" + name + '"');
    }
    return it->second;
}
