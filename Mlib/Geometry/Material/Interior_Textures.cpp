#include "Interior_Textures.hpp"
#include <Mlib/Geometry/Material/Interior_Texture_Set.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <ostream>
#include <stdexcept>

using namespace Mlib;

InteriorTextures::InteriorTextures()
    : set{ InteriorTextureSet::NONE }
{}

InteriorTextures::InteriorTextures(const InteriorTextures& other) = default;
InteriorTextures::InteriorTextures(InteriorTextures&& other) = default;
InteriorTextures& InteriorTextures::operator = (const InteriorTextures& other) = default;
InteriorTextures& InteriorTextures::operator = (InteriorTextures&& other) = default;

bool InteriorTextures::empty() const {
    return names.empty();
}

size_t InteriorTextures::size() const {
    return names.size();
}

const VariableAndHash<std::string>& InteriorTextures::operator [](size_t index) const {
    if (index >= names.size()) {
        THROW_OR_ABORT("Interior texture index out of bounds");
    }
    return names[index];
}

void InteriorTextures::assign(
    VariableAndHash<std::string> left,
    VariableAndHash<std::string> right,
    VariableAndHash<std::string> floor,
    VariableAndHash<std::string> ceiling,
    VariableAndHash<std::string> back,
    VariableAndHash<std::string> back_specular,
    VariableAndHash<std::string> front_color,
    VariableAndHash<std::string> front_alpha,
    VariableAndHash<std::string> front_specular)
{
    if (left->empty() || right->empty() || floor->empty() || ceiling->empty() + back->empty()) {
        THROW_OR_ABORT("Interior color texture is empty");
    }
    names.reserve(5 + !back_specular->empty() + !front_color->empty() + !front_alpha->empty() + !front_specular->empty());
    names.emplace_back(std::move(left));
    names.emplace_back(std::move(right));
    names.emplace_back(std::move(floor));
    names.emplace_back(std::move(ceiling));
    names.emplace_back(std::move(back));
    set |= InteriorTextureSet::INTERIOR_COLORS;
    if (!back_specular->empty()) {
        set |= InteriorTextureSet::BACK_SPECULAR;
        names.emplace_back(std::move(back_specular));
    }
    if (!front_color->empty()) {
        set |= InteriorTextureSet::FRONT_COLOR;
        names.emplace_back(std::move(front_color));
    }
    if (!front_alpha->empty()) {
        set |= InteriorTextureSet::FRONT_ALPHA;
        names.emplace_back(std::move(front_alpha));
    }
    if (!front_specular->empty()) {
        set |= InteriorTextureSet::FRONT_SPECULAR;
        names.emplace_back(std::move(front_specular));
    }
}

std::ostream& Mlib::operator << (std::ostream& ostr, const InteriorTextures& t) {
    ostr <<
        "facade_edge_size: " << t.facade_edge_size << '\n' <<
        "facade_inner_size: " << t.facade_inner_size << '\n' <<
        "interior_size: " << t.interior_size << '\n';
    for (size_t i = 0; i < t.size(); ++i) {
        ostr << i << ": " << *t[i] << '\n';
    }
    return ostr;
}
