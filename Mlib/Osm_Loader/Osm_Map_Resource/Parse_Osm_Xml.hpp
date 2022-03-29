#include <map>
#include <string>

namespace Mlib {

template <class TData, size_t n>
class TransformationMatrix;

struct Node;
struct Way;
class NormalizedPointsFixed;

void parse_osm_xml(
    const std::string& filename,
    float scale,
    NormalizedPointsFixed& normalized_points,
    TransformationMatrix<double, 2>& normalization_matrix,
    std::map<std::string, Node>& nodes,
    std::map<std::string, Way>& ways);

}
