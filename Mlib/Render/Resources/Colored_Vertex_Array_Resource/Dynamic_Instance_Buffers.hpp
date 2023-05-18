#pragma once
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Billboard_Sequence.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Dynamic_Billboard_Ids.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Dynamic_Position.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Dynamic_Position_YAngles.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/IInstance_Buffers.hpp>
#include <string>
#include <vector>

namespace Mlib {

template <class TDir, class TPos, size_t n>
class TransformationMatrix;
struct TransformationAndBillboardId;
enum class TransformationMode;

class DynamicInstanceBuffers: public IInstanceBuffers {
    DynamicInstanceBuffers(const DynamicInstanceBuffers&) = delete;
    DynamicInstanceBuffers& operator = (const DynamicInstanceBuffers&) = delete;
public:
    DynamicInstanceBuffers(
        TransformationMode transformation_mode,
        GLsizei max_num_instances,
        uint32_t num_billboard_atlas_components);
    virtual ~DynamicInstanceBuffers() override;

    void append(
        const TransformationMatrix<float, float, 3>& transformation_matrix,
        const BillboardSequence& sequence);
    void advance_time(float dt);
    GLsizei capacity() const;
    GLsizei length() const;
    bool empty() const;

    // IInstanceBuffers
    virtual void bind_position_yangles(GLuint attribute_index) const override;
    virtual void bind_position(GLuint attribute_index) const override;
    virtual void bind_billboard_atlas_instances(GLuint attribute_index) const override;
    virtual GLsizei num_instances() const override;
private:
    DynamicPositionYAngles position_yangles_;
    DynamicPosition position_;
    DynamicBillboardIds billboard_ids_;
    GLsizei num_instances_;
    TransformationMode transformation_mode_;
    std::vector<float> animation_times_;
    std::vector<const BillboardSequence*> billboard_sequences_;
};

}
