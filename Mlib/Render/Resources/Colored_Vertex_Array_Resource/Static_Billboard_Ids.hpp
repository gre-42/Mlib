#pragma once
#include <Mlib/Billboard_Id.hpp>
#include <Mlib/Render/Instance_Handles/Buffer_Background_Copy.hpp>
#include <vector>

namespace Mlib {

struct TransformationAndBillboardId;

class StaticBillboardIds {
    StaticBillboardIds(const StaticBillboardIds &) = delete;
    StaticBillboardIds &operator=(const StaticBillboardIds &) = delete;

public:
    StaticBillboardIds(const std::vector<TransformationAndBillboardId> &instances,
                       BillboardId num_billboard_atlas_components);
    ~StaticBillboardIds();
    bool copy_in_progress() const;
    void wait() const;
    void bind(GLuint attribute_index, TaskLocation task_location) const;

private:
    const std::vector<TransformationAndBillboardId> &instances_;
    BillboardId num_billboard_atlas_components_;
    std::vector<BillboardId> billboard_ids_;
    mutable BufferBackgroundCopy buffer_;
};

}
