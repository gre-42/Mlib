#pragma once
#include <iosfwd>
#include <string>

namespace Mlib {

struct NormalmapWithModifiers {
    std::string filename;
    std::string average = "";
    std::partial_ordering operator <=> (const NormalmapWithModifiers&) const = default;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(filename);
        archive(average);
    }
};

std::ostream& operator << (std::ostream& ostr, const NormalmapWithModifiers& t);

}
