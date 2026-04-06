#pragma once
#include <Mlib/Memory/Destruction_Functions.hpp>

namespace Mlib {

class DestructionNotifier {
public:
    mutable EarlyAndLateDestructionFunctions on_destroy;
};

}
