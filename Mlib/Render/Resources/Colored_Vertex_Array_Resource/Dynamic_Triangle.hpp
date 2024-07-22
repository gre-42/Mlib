#pragma once
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Render/Instance_Handles/IArray_Buffer.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Dynamic_Base.hpp>
#include <cstddef>

namespace Mlib {

class DynamicTriangle: public IArrayBuffer {
    DynamicTriangle(const DynamicTriangle&) = delete;
    DynamicTriangle& operator = (const DynamicTriangle&) = delete;
public:
    explicit DynamicTriangle(size_t max_num_triangles);
    ~DynamicTriangle();

    virtual bool copy_in_progress() const override;
    virtual void wait() const override;
    virtual void update() override;
    virtual void bind() const override;
    virtual bool is_awaited() const override;
    virtual std::shared_ptr<IArrayBuffer> fork() override;

    void append(const FixedArray<ColoredVertex<float>, 3>& t);
    void remove(size_t i);
protected:
    virtual void set_type_erased(
        const char* begin,
        const char* end,
        TaskLocation task_location) override;

private:
    DynamicBase<FixedArray<ColoredVertex<float>, 3>> data_;
};

}
