#pragma once
#include <Mlib/Map/String_With_Hash_Unordered_Map.hpp>
#include <Mlib/Physics/Bullets/Bullet_Properties.hpp>
#include <map>
#include <string>

namespace Mlib {

class BulletPropertyDb {
public:
    BulletPropertyDb();
    ~BulletPropertyDb();
    void add(VariableAndHash<std::string> name, BulletProperties&& props);
    const BulletProperties& get(const VariableAndHash<std::string>& name) const;
private:
    StringWithHashUnorderedMap<BulletProperties> properties_;
};

}
