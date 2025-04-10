#include "Load_Material.hpp"
#include <Mlib/Geometry/Mesh/Obj_Material.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Strings/To_Number.hpp>

using namespace Mlib;

std::map<std::string, ObjMaterial> Mlib::load_mtllib(const std::string& filename, bool werror)
{
    std::map<std::string, ObjMaterial> mtllib;

    auto ifs_p = create_ifstream(filename);
    auto& ifs = *ifs_p;
    if (ifs.fail()) {
        THROW_OR_ABORT("Could not open material file \"" + filename + '"');
    }

    static const DECLARE_REGEX(newmtl_reg, "^newmtl (.+)$");
    static const DECLARE_REGEX(Ka_reg, "^\\s*Ka +(\\S+) (\\S+) (\\S+)$");
    static const DECLARE_REGEX(Kd_reg, "^\\s*Kd +(\\S+) (\\S+) (\\S+)$");
    static const DECLARE_REGEX(Ks_reg, "^\\s*Ks +(\\S+) (\\S+) (\\S+)$");
    static const DECLARE_REGEX(Ke_reg, "^\\s*Ke +(\\S+) (\\S+) (\\S+)$");
    static const DECLARE_REGEX(Km_reg, "^\\s*Km .+$");
    static const DECLARE_REGEX(Ni_reg, "^\\s*Ni .+$");
    static const DECLARE_REGEX(Ns_reg, "^\\s*Ns (.+)$");
    static const DECLARE_REGEX(Tr_reg, "^\\s*Tr .+$");
    static const DECLARE_REGEX(Tf_reg, "^\\s*Tf .+$");
    static const DECLARE_REGEX(illum_reg, "^\\s*illum .+$");
    static const DECLARE_REGEX(d_reg, "^\\s*d (.+)$");
    static const DECLARE_REGEX(map_Ke_reg, "^\\s*map_Ke +(.+)$");
    static const DECLARE_REGEX(map_Kd_reg, "^\\s*map_Kd +(.+)$");
    static const DECLARE_REGEX(map_Ks_reg, "^\\s*map_Ks +(.+)$");
    static const DECLARE_REGEX(map_refl_reg, "^\\s*map_refl +(.+)$");
    static const DECLARE_REGEX(map_d_reg, "^\\s*map_d +(.+)$");
    static const DECLARE_REGEX(map_bump_reg, "^\\s*(?:map_Bump|bump) +(?:-bm \\S+ +)?(.+)$");
    static const DECLARE_REGEX(map_Ns_reg, "^\\s*map_Ns +(.+)$");
    static const DECLARE_REGEX(refl_reg, "^\\s*refl +(.+)$");
    static const DECLARE_REGEX(comment_reg, "^\\s*#.*$");

    std::string mtl;
    std::string line;
    while(std::getline(ifs, line)) {
        if (line.length() > 0 && line[line.length() - 1] == '\r') {
            line = line.substr(0, line.length() - 1);
        }
        if (line.length() == 0) {
            continue;
        }
        Mlib::re::cmatch match;
        if (Mlib::re::regex_match(line, match, newmtl_reg)) {
            mtl = match[1].str();
            if (!mtllib.try_emplace(mtl, ObjMaterial()).second) {
                if (werror) {
                    THROW_OR_ABORT("Redefinition of material \"" + mtl + '"');
                } else {
                    lerr() << "WARNING: Redefinition of material \"" + mtl + '"';
                }
            }
        } else if (Mlib::re::regex_match(line, match, comment_reg)) {
            // do nothing
        } else if (Mlib::re::regex_match(line, match, Ke_reg)) {
            mtllib.at(mtl).emissive = FixedArray<float, 3>{
                safe_stof(match[1].str()),
                safe_stof(match[2].str()),
                safe_stof(match[3].str())};
        } else if (Mlib::re::regex_match(line, match, Ka_reg)) {
            mtllib.at(mtl).ambient = FixedArray<float, 3>{
                safe_stof(match[1].str()),
                safe_stof(match[2].str()),
                safe_stof(match[3].str())};
        } else if (Mlib::re::regex_match(line, match, Kd_reg)) {
            mtllib.at(mtl).diffuse = FixedArray<float, 3>{
                safe_stof(match[1].str()),
                safe_stof(match[2].str()),
                safe_stof(match[3].str())};
        } else if (Mlib::re::regex_match(line, match, Ks_reg)) {
            mtllib.at(mtl).specular = FixedArray<float, 3>{
                safe_stof(match[1].str()),
                safe_stof(match[2].str()),
                safe_stof(match[3].str())};
        } else if (Mlib::re::regex_match(line, match, Ni_reg)) {
            // do nothing
        } else if (Mlib::re::regex_match(line, match, Ns_reg)) {
            mtllib.at(mtl).specular_exponent = safe_stof(match[1].str());
        } else if (Mlib::re::regex_match(line, match, Tr_reg)) {
            // do nothing
        } else if (Mlib::re::regex_match(line, match, Tf_reg)) {
            // do nothing
        } else if (Mlib::re::regex_match(line, match, d_reg)) {
            mtllib.at(mtl).alpha = safe_stof(match[1].str());
        } else if (Mlib::re::regex_match(line, match, illum_reg)) {
            // do nothing
        } else if (Mlib::re::regex_match(line, match, Km_reg)) {
            // do nothing
        } else if (Mlib::re::regex_match(line, match, map_Ke_reg)) {
            // do nothing
        } else if (Mlib::re::regex_match(line, match, map_Kd_reg)) {
            mtllib.at(mtl).color_texture = match[1].str();
        } else if (Mlib::re::regex_match(line, match, map_Ks_reg) ||
                   Mlib::re::regex_match(line, match, map_refl_reg))
        {
            mtllib.at(mtl).specular_texture = match[1].str();
        } else if (Mlib::re::regex_match(line, match, map_bump_reg)) {
            mtllib.at(mtl).bump_texture = match[1].str();
        } else if (Mlib::re::regex_match(line, match, map_Ns_reg)) {
            // do nothing
        } else if (Mlib::re::regex_match(line, match, refl_reg)) {
            // do nothing
        } else if (Mlib::re::regex_match(line, match, map_d_reg)) {
            if (match[1].str() != mtllib.at(mtl).color_texture) {
                THROW_OR_ABORT("map_d differs from map_Kd");
            }
            mtllib.at(mtl).has_alpha_texture = true;
        } else {
            THROW_OR_ABORT("Could not parse line " + line);
        }
    }
    if (!ifs.eof() && ifs.fail()) {
        THROW_OR_ABORT("Error reading from file " + filename);
    }
    return mtllib;
}
