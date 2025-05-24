#pragma once
#include <cstdint>
#include <string>

namespace Mlib {

template <class T>
class DanglingRef;
class SceneNode;

struct ImposterInfo {
    std::string debug_prefix;
    uint32_t max_texture_size;
};

class IImposters {
public:
    virtual void set_imposter_info(
        const DanglingRef<SceneNode>& scene_node,
        const ImposterInfo& info) = 0;
};

}
