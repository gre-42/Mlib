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
        if (all(material.ambience != -1.f)) {
            ostr << "Ka " << material.ambience << '\n';
        }
        if (all(material.ambience != -1.f)) {
            ostr << "Kd " << material.diffusivity << '\n';
        }
        if (all(material.ambience != -1.f)) {
            ostr << "Ks " << material.specularity << '\n';
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
        throw std::runtime_error("Could not write to file \"" + filename + '"');
    }
}
