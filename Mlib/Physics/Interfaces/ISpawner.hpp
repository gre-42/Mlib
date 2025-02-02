#pragma once
#include <string>

namespace Mlib {

class IPlayer;
template <class T>
class DanglingBaseClassPtr;

class ISpawner {
public:
    virtual DanglingBaseClassPtr<IPlayer> player() = 0;
};

}
