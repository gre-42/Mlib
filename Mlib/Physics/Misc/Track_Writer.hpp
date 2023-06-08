#pragma once
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <iosfwd>
#include <memory>
#include <string>

namespace Mlib {

struct TrackElement;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;

class TrackWriter {
public:
    TrackWriter(
        const std::string& filename,
        const TransformationMatrix<double, double, 3>* geographic_mapping);
    ~TrackWriter();
    void write(const TrackElement& e);
    void flush();
private:
    std::string filename_;
    const TransformationMatrix<double, double, 3>* geographic_mapping_;
    std::unique_ptr<std::ostream> ofstr_;
};

}
