#pragma once
#include <Mlib/Render/Instance_Handles/Buffer_Background_Copy.hpp>
#include <vector>

namespace Mlib {

struct TransformationAndBillboardId;

class StaticBillboardIds {
    StaticBillboardIds(const StaticBillboardIds &) = delete;
    StaticBillboardIds &operator=(const StaticBillboardIds &) = delete;

public:
    StaticBillboardIds(const std::vector<TransformationAndBillboardId> &instances,
                       uint32_t num_billboard_atlas_components);
    ~StaticBillboardIds();
    bool copy_in_progress() const;
    void wait() const;
    void bind(GLuint attribute_index) const;

private:
    using BillboardId = uint32_t;
    const std::vector<TransformationAndBillboardId> &instances_;
    uint32_t num_billboard_atlas_components_;
    std::vector<BillboardId> billboard_ids_;
    mutable BufferBackgroundCopy buffer_;
};

}
