#include "Modifier_Backlog.hpp"
#include <sstream>

using namespace Mlib;

std::string Mlib::modifier_backlog_to_string(const ModifierBacklog& modifier_backlog) {
    std::stringstream sstr;
    sstr << "merge_textures=" << (int)modifier_backlog.merge_textures << ", " <<
        "convert_to_terrain=" << (int)modifier_backlog.convert_to_terrain << ", " <<
        "add_foliage=" << (int)modifier_backlog.add_foliage;
    return sstr.str();
}
