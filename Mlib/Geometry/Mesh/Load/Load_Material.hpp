#pragma once
#include <map>
#include <string>

namespace Mlib {

struct ObjMaterial;

std::map<std::string, ObjMaterial> load_mtllib(const std::string& filename, bool werror);

}
