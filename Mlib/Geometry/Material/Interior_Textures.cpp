#include "Interior_Textures.hpp"
#include <Mlib/Regex_Select.hpp>
#include <ostream>
#include <stdexcept>

using namespace Mlib;

bool InteriorTextures::empty() const {
    size_t nempty =
        (size_t)left.empty() +
        (size_t)right.empty() +
        (size_t)floor.empty() +
        (size_t)ceiling.empty() +
        (size_t)back.empty();
    if (nempty == 0) {
        return false;
    } else if (nempty == 5) {
        return true;
    } else {
        throw std::runtime_error("Inconsistent interior texture emptiness");
    }
}

const std::string& InteriorTextures::operator [](size_t index) const {
    switch (index) {
        case INTERIOR_LEFT: return left;
        case INTERIOR_RIGHT: return right;
        case INTERIOR_FLOOR: return floor;
        case INTERIOR_CEILING: return ceiling;
        case INTERIOR_BACK: return back;
        default: throw std::runtime_error("Interior texture index out of bounds");
    }
}

InteriorTextures InteriorTextures::parse(const std::string& text) {
    static const DECLARE_REGEX(re,
        "^\\s*"
        "left:(\\S+)"
        "\\s+right:(\\S+)"
        "\\s+floor:(\\S+)"
        "\\s+ceiling:(\\S+)"
        "\\s+back:(\\S+)"
        "\\s*$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(text, match, re)) {
        return InteriorTextures{
            .left = match[1].str(),
            .right = match[2].str(),
            .floor = match[3].str(),
            .ceiling = match[4].str(),
            .back = match[5].str(),
        };
    } else {
        throw std::runtime_error("Could not parse interior textures \"" + text + '"');
    }
}

std::ostream& Mlib::operator << (std::ostream& ostr, const InteriorTextures& t) {
    ostr <<
        "left: " << t.left << '\n' <<
        "right: " << t.right << '\n' <<
        "floor: " << t.floor << '\n' <<
        "ceiling: " << t.ceiling << '\n' <<
        "back: " << t.back << '\n';
    return ostr;
}
