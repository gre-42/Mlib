
#include "Bullet_Property_Db.hpp"
#include <stdexcept>

using namespace Mlib;

BulletPropertyDb::BulletPropertyDb()
    : properties_{ "Bullet" }
{}

BulletPropertyDb::~BulletPropertyDb() = default;

void BulletPropertyDb::add(VariableAndHash<std::string> name, BulletProperties&& props) {
    if (!properties_.try_emplace(std::move(name), std::move(props)).second) {
        throw std::runtime_error("Bullet properties with name \"" + *name + "\" already exist");
    }
}

const BulletProperties& BulletPropertyDb::get(const VariableAndHash<std::string>& name) const {
    auto it = properties_.find(name);
    if (it == properties_.end()) {
        throw std::runtime_error("Could not find bullet properties with name \"" + *name + '"');
    }
    return it->second;
}
