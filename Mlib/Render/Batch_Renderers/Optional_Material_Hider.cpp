#include "Optional_Material_Hider.hpp"
#include <Mlib/Env.hpp>
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Os/Os.hpp>

using namespace Mlib;

OptionalMaterialHider::OptionalMaterialHider() {
    print_materials_ = getenv_default_bool("PRINT_MATERIALS", false);
    const char* fn = getenv("HIDDEN_MATERIALS");
    if (fn != nullptr) {
        hidden_names_.emplace();
        auto f = create_ifstream(fn);
        for (std::string line; std::getline(*f, line); )
        {
            hidden_names_.value().insert(line);
        }
    }
}

bool OptionalMaterialHider::is_hidden(const Material& material) const {
    if (!hidden_names_.has_value()) {
        return false;
    }
    if (material.textures.empty()) {
        return false;
    }
    bool result = hidden_names_.value().contains(
        material.textures[0].texture_descriptor.color);
    if (print_materials_ && !result) {
        linfo() << "Drawing material " << material.identifier();
    }
    return result;
}
