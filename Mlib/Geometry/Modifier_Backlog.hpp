#pragma once
#include <compare>
#include <string>

namespace Mlib {

struct ModifierBacklog {
    bool merge_textures = false;
    bool convert_to_terrain = false;
    bool add_foliage = false;
    std::strong_ordering operator <=> (const ModifierBacklog&) const = default;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(merge_textures);
        archive(convert_to_terrain);
        archive(add_foliage);
    }
};

std::string modifier_backlog_to_string(const ModifierBacklog& modifier_backlog);

}
