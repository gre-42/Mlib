#include "Load_Material.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Strings/From_Number.hpp>
#include <regex>

using namespace Mlib;

std::map<std::string, ObjMaterial> Mlib::load_mtllib(const std::string& filename, bool werror)
{
    std::map<std::string, ObjMaterial> mtllib;

    std::ifstream ifs{filename};

    static const DECLARE_REGEX(newmtl_reg, "^newmtl (.+)$");
    static const DECLARE_REGEX(Ka_reg, "^\\s*Ka +(\\S+) (\\S+) (\\S+)$");
    static const DECLARE_REGEX(Kd_reg, "^\\s*Kd +(\\S+) (\\S+) (\\S+)$");
    static const DECLARE_REGEX(Ks_reg, "^\\s*Ks +(\\S+) (\\S+) (\\S+)$");
    static const DECLARE_REGEX(Ke_reg, "^\\s*Ke .+$");
    static const DECLARE_REGEX(Km_reg, "^\\s*Km .+$");
    static const DECLARE_REGEX(Ni_reg, "^\\s*Ni .+$");
    static const DECLARE_REGEX(Ns_reg, "^\\s*Ns .+$");
    static const DECLARE_REGEX(Tr_reg, "^\\s*Tr .+$");
    static const DECLARE_REGEX(Tf_reg, "^\\s*Tf .+$");
    static const DECLARE_REGEX(illum_reg, "^\\s*illum .+$");
    static const DECLARE_REGEX(d_reg, "^\\s*d .+$");
    static const DECLARE_REGEX(map_Kd_reg, "^\\s*map_Kd (.+)$");
    static const DECLARE_REGEX(map_Ks_reg, "^\\s*map_Ks (.+)$");
    static const DECLARE_REGEX(map_d_reg, "^\\s*map_d (.+)$");
    static const DECLARE_REGEX(map_bump_reg, "^\\s*map_Bump (.+)$");
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
        Mlib::re::smatch match;
        if (Mlib::re::regex_match(line, match, newmtl_reg)) {
            mtl = match[1].str();
            auto res = mtllib.insert(std::make_pair(mtl, ObjMaterial()));
            if (!res.second) {
                if (werror) {
                    throw std::runtime_error("Redefinition of material \"" + mtl + '"');
                } else {
                    std::cerr << "WARNING: Redefinition of material \"" + mtl + '"' << std::endl;
                }
            }
        } else if (Mlib::re::regex_match(line, match, comment_reg)) {
            // do nothing
        } else if (Mlib::re::regex_match(line, match, Ka_reg)) {
            mtllib.at(mtl).ambience = FixedArray<float, 3>{
                safe_stof(match[1].str()),
                safe_stof(match[2].str()),
                safe_stof(match[3].str())};
        } else if (Mlib::re::regex_match(line, match, Kd_reg)) {
            mtllib.at(mtl).diffusivity = FixedArray<float, 3>{
                safe_stof(match[1].str()),
                safe_stof(match[2].str()),
                safe_stof(match[3].str())};
        } else if (Mlib::re::regex_match(line, match, Ks_reg)) {
            mtllib.at(mtl).specularity = FixedArray<float, 3>{
                safe_stof(match[1].str()),
                safe_stof(match[2].str()),
                safe_stof(match[3].str())};
        } else if (Mlib::re::regex_match(line, match, Ni_reg)) {
            // do nothing
        } else if (Mlib::re::regex_match(line, match, Ns_reg)) {
            // do nothing
        } else if (Mlib::re::regex_match(line, match, Tr_reg)) {
            // do nothing
        } else if (Mlib::re::regex_match(line, match, Tf_reg)) {
            // do nothing
        } else if (Mlib::re::regex_match(line, match, d_reg)) {
            // do nothing
        } else if (Mlib::re::regex_match(line, match, illum_reg)) {
            // do nothing
        } else if (Mlib::re::regex_match(line, match, Ke_reg)) {
            // do nothing
        } else if (Mlib::re::regex_match(line, match, Km_reg)) {
            // do nothing
        } else if (Mlib::re::regex_match(line, match, map_Kd_reg)) {
            mtllib.at(mtl).color_texture = match[1].str();
        } else if (Mlib::re::regex_match(line, match, map_Ks_reg)) {
            // do nothing
        } else if (Mlib::re::regex_match(line, match, map_bump_reg)) {
            mtllib.at(mtl).bump_texture = match[1].str();
        } else if (Mlib::re::regex_match(line, match, map_d_reg)) {
            if (match[1].str() != mtllib.at(mtl).color_texture) {
                throw std::runtime_error("map_d differs from map_Kd");
            }
            mtllib.at(mtl).has_alpha_texture = true;
        } else {
            throw std::runtime_error("Could not parse line " + line);
        }
    }
    if (!ifs.eof() && ifs.fail()) {
        throw std::runtime_error("Error reading from file " + filename);
    }
    return mtllib;
}
