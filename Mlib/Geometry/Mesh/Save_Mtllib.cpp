#include "Save_Mtllib.hpp"
#include <Mlib/Geometry/Mesh/Obj_Material.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <fstream>

using namespace Mlib;

void Mlib::save_mtllib(
    const std::string& filename,
    const std::map<std::string, ObjMaterial>& materials)
{
    std::ofstream ostr(filename);
    ostr.precision(15);
    ostr << std::scientific;
    for (const auto& [name, material] : materials) {
        ostr << "newmtl " << name << '\n';
        if (all(material.ambient != -1.f)) {
            ostr << "Ka " << material.ambient << '\n';
        }
        if (all(material.diffuse != -1.f)) {
            ostr << "Kd " << material.diffuse << '\n';
        }
        if (all(material.specular != -1.f)) {
            ostr << "Ks " << material.specular << '\n';
        }
        if (!material.color_texture.empty()) {
            ostr << "map_Kd " << material.color_texture << '\n';
        }
        if (!material.bump_texture.empty()) {
            ostr << "map_Bump " << material.bump_texture << '\n';
        }
        if (material.has_alpha_texture) {
            ostr << "map_d " << material.color_texture << '\n';
        }
    }
    ostr.flush();
    if (ostr.fail()) {
        THROW_OR_ABORT("Could not write to file \"" + filename + '"');
    }
}
