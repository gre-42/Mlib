#pragma once

namespace Mlib {

struct ModifierBacklog {
    bool merge_textures = false;
    bool convert_to_terrain = false;
    bool add_foliage = false;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(merge_textures);
        archive(convert_to_terrain);
        archive(add_foliage);
    }
};

}
