#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Physics/Misc/Track_Element.hpp>
#include <fstream>

namespace Mlib {

template <class TDir, class TPos, size_t n>
class TransformationMatrix;

class TrackReader {
public:
    explicit TrackReader(
        const std::string& filename,
        bool periodic,
        const TransformationMatrix<double, double, 3>* inverse_geographic_mapping);
    bool read(TrackElement& track_element, size_t& nperiods, float dt);
    bool eof() const;
    void restart();
private:
    std::ifstream ifstr_;
    std::string filename_;
    bool periodic_;
    const TransformationMatrix<double, double, 3>* inverse_geographic_mapping_;
    float elapsed_seconds_;
    TrackElement track_element0_;
    TrackElement track_element1_;
};

}
