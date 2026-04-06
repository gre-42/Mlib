#include "Tile_Image_File.hpp"
#include <Mlib/Json/Json_View.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Misc/FPath_Json.hpp>
#include <Mlib/Os/Os.hpp>
#include <stdexcept>

using namespace Mlib;

namespace OlsCoefficientArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(offset);
DECLARE_ARGUMENT(slope);
}

namespace FragmentArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(color);
DECLARE_ARGUMENT(alpha);
DECLARE_ARGUMENT(alpha_fac);
DECLARE_ARGUMENT(size);
DECLARE_ARGUMENT(stepsize);
DECLARE_ARGUMENT(randsize);
DECLARE_ARGUMENT(channels);
DECLARE_ARGUMENT(add);
DECLARE_ARGUMENT(upsampling);
DECLARE_ARGUMENT(ols);
}

void Mlib::to_json(nlohmann::json& j, const OlsCoefficient& c) {
    j[OlsCoefficientArgs::offset] = c.offset;
    j[OlsCoefficientArgs::slope] = c.slope;
}

void Mlib::from_json(const nlohmann::json& j, OlsCoefficient& c) {
    JsonView jv{j};
    jv.validate(OlsCoefficientArgs::options);
    c.offset = jv.at<float>(OlsCoefficientArgs::offset);
    c.slope = jv.at<float>(OlsCoefficientArgs::slope);
}

void Mlib::to_json(nlohmann::json& j, const FragmentAssembly& fa) {
    j[FragmentArgs::color] = fa.color.string();
    if (!fa.alpha.empty()) {
        j[FragmentArgs::alpha] = fa.alpha.string();
    }
    if (!fa.alpha_fac.empty()) {
        j[FragmentArgs::alpha_fac] = fa.alpha_fac.string();
    }
    j[FragmentArgs::size] = fa.size;
    j[FragmentArgs::stepsize] = fa.stepsize;
    j[FragmentArgs::randsize] = fa.randsize;
    j[FragmentArgs::channels] = fa.channels;
    j[FragmentArgs::add] = fa.add;
    j[FragmentArgs::upsampling] = fa.upsampling;
    if (fa.ols.has_value()) {
        j[FragmentArgs::ols] = *fa.ols;
    }
}

void Mlib::from_json(const nlohmann::json& j, FragmentAssembly& fa) {
    JsonView jv{j};
    jv.validate(FragmentArgs::options);
    fa.color = jv.at<FPath>(FragmentArgs::color);
    if (auto a = jv.try_at_non_null<FPath>(FragmentArgs::alpha)) fa.alpha = *a;
    if (auto a = jv.try_at_non_null<FPath>(FragmentArgs::alpha_fac)) fa.alpha_fac = *a;
    jv.at(FragmentArgs::size).get_to(fa.size);
    jv.at(FragmentArgs::stepsize).get_to(fa.stepsize);
    jv.at(FragmentArgs::randsize).get_to(fa.randsize);
    jv.at(FragmentArgs::channels).get_to(fa.channels);
    jv.at(FragmentArgs::add).get_to(fa.add);
    jv.at(FragmentArgs::upsampling).get_to(fa.upsampling);
    if (jv.contains(FragmentArgs::ols)) {
        fa.ols = jv.at<std::vector<OlsCoefficient>>(FragmentArgs::ols);
    }
}

void Mlib::save_fragment_assembly(const std::filesystem::path& filename, const FragmentAssembly& fa) {
    auto f = create_ofstream(filename);
    if (f->fail()) {
        throw std::runtime_error("Could not open for write: \"" + filename.string() + '"');
    }
    *f << nlohmann::json(fa).dump(4);
    f->flush();
    if (f->fail()) {
        throw std::runtime_error("Could not write to file: \"" + filename.string() + '"');
    }
}

FragmentAssembly Mlib::load_fragment_assembly(const std::filesystem::path& filename) {
    auto f = create_ifstream(filename);
    if (f->fail()) {
        throw std::runtime_error("Could not open for read: \"" + filename.string() + '"');
    }
    nlohmann::json j;
    *f >> j;
    if (f->fail()) {
        throw std::runtime_error("Could not read from file: \"" + filename.string() + '"');
    }
    auto res = j.get<FragmentAssembly>();
    res.make_pathes_absolute(filename);
    return res;
}

void FragmentAssembly::make_pathes_absolute(const std::filesystem::path& filename) {
    color = FPath::from_local_path(filename.parent_path() / color.local_path());
    if (!alpha.empty()) {
        alpha = FPath::from_local_path(filename.parent_path() / alpha.local_path());
    }
    if (!alpha_fac.empty()) {
        alpha_fac = FPath::from_local_path(filename.parent_path() / alpha_fac.local_path());
    }
}
