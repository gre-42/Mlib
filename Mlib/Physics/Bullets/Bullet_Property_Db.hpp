#include <Mlib/Physics/Bullets/Bullet_Properties.hpp>
#include <map>
#include <string>

namespace Mlib {

class BulletPropertyDb {
public:
    BulletPropertyDb();
    ~BulletPropertyDb();
    void add(std::string name, BulletProperties&& props);
    const BulletProperties& get(const std::string& name);
private:
    std::map<std::string, BulletProperties> properties_;
};

}
