#pragma once
#include <string>

namespace Mlib {

class IPlayer;

class ISpawner {
public:
    virtual IPlayer* player() = 0;
};

}
