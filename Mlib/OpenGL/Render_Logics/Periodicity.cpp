
#include "Periodicity.hpp"
#include <map>
#include <stdexcept>

using namespace Mlib;

Periodicity Mlib::periodicity_from_string(const std::string& s) {
    const std::map<std::string, Periodicity> m{
        {"periodic", Periodicity::PERIODIC},
        {"aperiodic", Periodicity::APERIODIC},
    };
    auto it = m.find(s);
    if (it == m.end()) {
        throw std::runtime_error("Unknown periodicity: \"" + s + '"');
    }
    return it->second;
}
