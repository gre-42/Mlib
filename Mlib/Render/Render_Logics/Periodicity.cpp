#include "Periodicity.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <map>

using namespace Mlib;

Periodicity Mlib::periodicity_from_string(const std::string& s) {
    const std::map<std::string, Periodicity> m{
        {"periodic", Periodicity::PERIODIC},
        {"aperiodic", Periodicity::APERIODIC},
    };
    auto it = m.find(s);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown periodicity: \"" + s + '"');
    }
    return it->second;
}
