#include "Optional_Mesh_Hider.hpp"
#include <Mlib/Env.hpp>
#include <Mlib/Os/Os.hpp>

using namespace Mlib;

OptionalMeshHider::OptionalMeshHider() {
    print_names_ = getenv_default_bool("PRINT_MESHES", false);
    const char* fn = getenv("HIDDEN_MESHES");
    if (fn != nullptr) {
        hidden_names_.emplace();
        auto f = create_ifstream(fn);
        for (std::string line; std::getline(*f, line); )
        {
            hidden_names_.value().insert(line);
        }
    }
}

bool OptionalMeshHider::is_hidden(const std::string& name) const {
    if (!hidden_names_.has_value()) {
        return false;
    }
    bool result = hidden_names_.value().contains(name);
    if (print_names_ && !result) {
        linfo() << "Drawing mesh " << name;
    }
    return result;
}
