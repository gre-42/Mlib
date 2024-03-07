#pragma once
#include <Mlib/Render/Instance_Handles/IArray_Buffer.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Dynamic_Base.hpp>
#include <cstddef>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
template <class TPos>
struct ColoredVertex;

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

    void append(const FixedArray<ColoredVertex<float>, 3>& t);
    void remove(size_t i);
protected:
    virtual void set_type_erased(const char* begin, const char* end) override;

private:
    DynamicBase<FixedArray<ColoredVertex<float>, 3>> data_;
};

}
