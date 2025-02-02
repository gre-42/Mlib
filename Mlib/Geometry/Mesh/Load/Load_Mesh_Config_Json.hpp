#include <string>

namespace Mlib {

class JsonMacroArguments;
template <class TPos>
struct LoadMeshConfig;

template <class TPos>
LoadMeshConfig<TPos> load_mesh_config_from_json(const JsonMacroArguments& j);

}
