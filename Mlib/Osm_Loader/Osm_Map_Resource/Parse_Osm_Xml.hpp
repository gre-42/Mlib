#include <map>
#include <optional>
#include <string>

namespace Mlib {

template <class TDir, class TPos, size_t n>
class TransformationMatrix;

struct Node;
struct Way;
template <class TData>
class NormalizedPointsFixed;

void parse_osm_xml(
    const std::string& filename,
    double scale,
    NormalizedPointsFixed<double>& normalized_points,
    std::optional<TransformationMatrix<double, double, 2>>& normalization_matrix,
    std::map<std::string, Node>& nodes,
    std::map<std::string, Way>& ways);

}
