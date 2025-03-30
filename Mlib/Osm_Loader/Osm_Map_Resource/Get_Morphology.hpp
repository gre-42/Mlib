#pragma once
#include <Mlib/Geometry/Morphology.hpp>

namespace Mlib {

struct Morphology;
enum class BuildingDetailType;

class GetMorphology {
public:
    GetMorphology(
        const Morphology& high_detail_morphology,
        const Morphology& low_detail_morphology,
        const Morphology& combined_detail_morphology);

    const Morphology& operator[](BuildingDetailType detail_type) const;
private:
    Morphology high_detail_morphology_;
    Morphology low_detail_morphology_;
    Morphology combined_detail_morphology_;
};

}
