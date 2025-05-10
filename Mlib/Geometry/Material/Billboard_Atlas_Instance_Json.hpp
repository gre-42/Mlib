#include <nlohmann/json_fwd.hpp>

namespace Mlib {

struct BillboardAtlasInstance;

void from_json(const nlohmann::json& j, BillboardAtlasInstance& bb);

}
