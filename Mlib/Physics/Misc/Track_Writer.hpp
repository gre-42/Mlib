#pragma once
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <fstream>
#include <string>

namespace Mlib {

struct TrackElement;
template <class TData, size_t n>
class TransformationMatrix;

class TrackWriter {
public:
    TrackWriter(
        const std::string& filename,
        const TransformationMatrix<double, 3>* geographic_mapping);
    void write(const TrackElement& e);
    void flush();
private:
    std::string filename_;
    const TransformationMatrix<double, 3>* geographic_mapping_;
    std::ofstream ofstr_;
};

}
