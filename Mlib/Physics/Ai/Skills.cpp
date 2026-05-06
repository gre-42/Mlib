#include "Skills.hpp"
#include <cereal/archives/binary.hpp>

using namespace Mlib;

void Skills::write(std::ostream& ostr) const {
    cereal::BinaryOutputArchive oarchive(ostr);
    oarchive(*this);
}

Skills& Skills::read(std::istream& istr) {
    cereal::BinaryInputArchive iarchive(istr);
    iarchive(*this);
    return *this;
}
