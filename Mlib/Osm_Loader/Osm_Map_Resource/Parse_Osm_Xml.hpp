#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Coordinates/Normalized_Points_Fixed.hpp>
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <map>
#include <optional>
#include <string>

namespace Mlib {

template <class TDir, class TPos, size_t n>
class TransformationMatrix;

struct Node;
struct Way;

class OsmBounds {
public:
    explicit OsmBounds(double scale);
    void extend(const FixedArray<double, 2>& bounds_min, const FixedArray<double, 2>& bounds_max);
    const NormalizedPointsFixed<double>& normalized_points() const;
    const TransformationMatrix<double, double, 2>& normalization_matrix() const;
    inline const AxisAlignedBoundingBox<double, 2>& aabb() const {
        return bounds_merged_;
    }
    inline double scale() const {
        return scale_;
    }
private:
    double scale_;
    AxisAlignedBoundingBox<double, 2> bounds_merged_;
    mutable bool result_obtained_;
    std::optional<TransformationMatrix<double, double, 2>> normalization_matrix_;
    NormalizedPointsFixed<double> normalized_points_;
};

void parse_osm_xml(
    const std::string& filename,
    OsmBounds& bounds,
    std::map<std::string, Node>& nodes,
    std::map<std::string, Way>& ways);

}
