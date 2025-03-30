#include "Get_Morphology.hpp"
#include <Mlib/Osm_Loader/Osm_Map_Resource/Building.hpp>

using namespace Mlib;

GetMorphology::GetMorphology(
    const Morphology& high_detail_morphology,
    const Morphology& low_detail_morphology,
    const Morphology& combined_detail_morphology)
    : high_detail_morphology_{ high_detail_morphology }
    , low_detail_morphology_{ low_detail_morphology }
    , combined_detail_morphology_{ combined_detail_morphology }
{}

const Morphology& GetMorphology::operator[](BuildingDetailType detail_type) const {
    switch (detail_type) {
        case BuildingDetailType::HIGH:
            return high_detail_morphology_;
        case BuildingDetailType::LOW:
            return low_detail_morphology_;
        case BuildingDetailType::COMBINED:
            return combined_detail_morphology_;
        case BuildingDetailType::UNDEFINED:
            THROW_OR_ABORT("Building detail type is \"undefined\"");
    }
    THROW_OR_ABORT("Unknown building detail type: " + std::to_string((int)detail_type));
}
