#include "Load_Dff_Array.hpp"
#include <Mlib/Geometry/Mesh/Load/Load_Dff.hpp>

using namespace Mlib;

std::map<std::string, std::shared_ptr<ColoredVertexArray<float>>> Mlib::load_dff(
    const std::string& filename,
    const LoadMeshConfig<float>& cfg)
{
    auto ifs = create_ifstream(filename, std::ios::binary);
    if (ifs->fail()) {
        THROW_OR_ABORT("Could not open file for read: \"" + filename + '"');
    }
    try {
        return load_dff(*ifs, cfg);
    } catch (std::runtime_error& e) {
        THROW_OR_ABORT("Could not read file \"" + filename + "\": "+ e.what());
    }
}

std::map<std::string, std::shared_ptr<ColoredVertexArray<float>>> Mlib::load_dff(
    std::istream& istr,
    const LoadMeshConfig<float>& cfg)
{
    std::map<std::string, std::shared_ptr<ColoredVertexArray<float>>> result;
    auto clump = Mlib::Dff::read_dff(istr);
    return result;
}
