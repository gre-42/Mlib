#pragma once
#include <Mlib/Os/Io/Safe_Archiver.hpp>
#include <compare>
#include <string>

namespace Mlib {

struct ModifierBacklog {
    bool merge_textures = false;
    bool convert_to_terrain = false;
    bool add_foliage = false;
    std::strong_ordering operator <=> (const ModifierBacklog&) const = default;
    template <class Archive>
    void serialize(Archive& archiver) {
        SafeArchiver archive{archiver};
        archive(merge_textures);
        archive(convert_to_terrain);
        archive(add_foliage);
    }
};

std::string modifier_backlog_to_string(const ModifierBacklog& modifier_backlog);

}
