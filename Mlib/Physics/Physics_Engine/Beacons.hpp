#pragma once
#include <Mlib/Physics/Misc/Beacon.hpp>
#include <list>

namespace Mlib {

void add_beacon(Beacon beacon);
std::list<Beacon>& get_beacons();

}
