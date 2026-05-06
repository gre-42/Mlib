#include "Beacons.hpp"
#include <Mlib/Threads/Thread_Local.hpp>

using namespace Mlib;

void Mlib::add_beacon(Beacon beacon) {
    get_beacons().push_back(std::move(beacon));
}

std::list<Beacon>& Mlib::get_beacons() {
    using Beacons = std::list<Beacon>;
    static THREAD_LOCAL(Beacons) bcs = Beacons{};
    return bcs;
}
