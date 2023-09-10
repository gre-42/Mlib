#include "Merged_Texture_Name.hpp"
#include <Mlib/Geometry/Material.hpp>

using namespace Mlib;

static const std::string& get_name(const Material& material) {
    if (material.textures.size() != 1) {
        THROW_OR_ABORT("Material \"" + material.identifier() + "\" does not have exactly one texture");
    }
    return material.textures[0].texture_descriptor.color.filename;
}

MergedTextureName::MergedTextureName(const Material& material)
: name{get_name(material)}
{}
