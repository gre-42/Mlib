#include "Skills.hpp"
#include <Mlib/Os/Io/Serialize/Serialize.hpp>

using namespace Mlib;

void Skills::write(std::ostream& ostr) const {
    BinaryBitwiseWordsWriter writer{ostr, nullptr};
    WritingArchive oarchive{writer, "skills"};
    oarchive(*this);
}

Skills& Skills::read(std::istream& istr) {
    BinaryBitwiseWordsReader reader{istr, nullptr, IoVerbosity::SILENT};
    ReadingArchive iarchive{reader, "skills"};
    iarchive(*this);
    return *this;
}
