#include "Surface_Contact_Info.hpp"

using namespace Mlib;

SurfaceSmokeAffinity Mlib::surface_smoke_affinity_from_string(const std::string& s) {
    static const std::map<std::string, SurfaceSmokeAffinity> m{
        {"pair", SurfaceSmokeAffinity::PAIR},
        {"tire", SurfaceSmokeAffinity::TIRE}
    };
    auto it = m.find(s);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown surface smoke affinity: \"" + s + '"');
    }
    return it->second;
}
