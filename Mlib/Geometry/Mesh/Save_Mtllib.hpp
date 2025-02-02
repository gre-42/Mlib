#pragma once
#include <map>
#include <string>

namespace Mlib {

struct ObjMaterial;

void save_mtllib(
    const std::string& filename,
    const std::map<std::string, ObjMaterial>& materials);
}
