#include "Interior_Textures.hpp"
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <ostream>
#include <stdexcept>

using namespace Mlib;

InteriorTextures::InteriorTextures() = default;
InteriorTextures::InteriorTextures(const InteriorTextures& other) = default;
InteriorTextures::InteriorTextures(InteriorTextures&& other) = default;
InteriorTextures& InteriorTextures::operator = (const InteriorTextures& other) = default;
InteriorTextures& InteriorTextures::operator = (InteriorTextures&& other) = default;

bool InteriorTextures::empty() const {
    size_t nempty =
        (size_t)left->empty() +
        (size_t)right->empty() +
        (size_t)floor->empty() +
        (size_t)ceiling->empty() +
        (size_t)back->empty();
    if (nempty == 0) {
        return false;
    } else if (nempty == 5) {
        return true;
    } else {
        THROW_OR_ABORT("Inconsistent interior texture emptiness");
    }
}

const VariableAndHash<std::string>& InteriorTextures::operator [](size_t index) const {
    switch (index) {
        case INTERIOR_LEFT: return left;
        case INTERIOR_RIGHT: return right;
        case INTERIOR_FLOOR: return floor;
        case INTERIOR_CEILING: return ceiling;
        case INTERIOR_BACK: return back;
        default: THROW_OR_ABORT("Interior texture index out of bounds");
    }
}

std::ostream& Mlib::operator << (std::ostream& ostr, const InteriorTextures& t) {
    ostr <<
        "facade_edge_size: " << t.facade_edge_size << '\n' <<
        "facade_inner_size: " << t.facade_inner_size << '\n' <<
        "interior_size: " << t.interior_size << '\n' <<
        "left: " << *t.left << '\n' <<
        "right: " << *t.right << '\n' <<
        "floor: " << *t.floor << '\n' <<
        "ceiling: " << *t.ceiling << '\n' <<
        "back: " << *t.back << '\n';
    return ostr;
}
