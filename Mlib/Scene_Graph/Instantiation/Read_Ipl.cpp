#include "Read_Ipl.hpp"
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Transformation/Quaternion.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Regex/Template_Regex.hpp>
#include <Mlib/Scene_Graph/Instantiation/Instance_Information.hpp>
#include <Mlib/Stats/Mean.hpp>
#include <Mlib/Strings/RGetline.hpp>
#include <Mlib/Strings/String_View_To_Number.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;
using namespace Mlib::TemplateRegex;

std::list<InstanceInformation> Mlib::read_ipl(
    const std::filesystem::path& filename,
    RenderingDynamics rendering_dynamics)
{
    auto istr = create_ifstream(filename, std::ios::binary);
    if (istr->fail()) {
        THROW_OR_ABORT("Could not open \"" + filename.string() + '"');
    }
    try {
        return read_ipl(*istr, rendering_dynamics);
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("Could not read \"" + filename.string() + "\": " + e.what());
    }
}

std::list<InstanceInformation> Mlib::read_ipl(
    std::istream& istr,
    RenderingDynamics rendering_dynamics)
{
    {
        std::string header;
        rgetline(istr, header);
        if (istr.fail()) {
            THROW_OR_ABORT("Could not read ipl header");
        }
        if (!header.starts_with("#")) {
            THROW_OR_ABORT("Unexpected ipl header");
        }
    }
    {
        std::string inst;
        rgetline(istr, inst);
        if (istr.fail()) {
            THROW_OR_ABORT("Could not read ipl inst");
        }
        if (inst != "inst") {
            THROW_OR_ABORT("Unexpected ipl type");
        }
    }
    std::list<InstanceInformation> result;
    while (true) {
        std::string line;
        rgetline(istr, line);
        if (istr.fail()) {
            THROW_OR_ABORT("Could not read line");
        }
        if (line.empty()) {
            continue;
        }
        if (line == "end") {
            break;
        }
        static const auto c = str(", ");
        static const auto n = group(plus(CharPredicate{[](char c){return c != ',';}}));
        static const auto reg = seq(n, c, n, c, n, c, n, c, n, c, n, c, n, c, n, c, n, c, n, c, n, c, n, eof);

        SMatch<13> match;
        if (!regex_match(line, match, reg)) {
            THROW_OR_ABORT("Could not parse line \"" + line + '"');
        }
        std::string name{ match[2].str() };
        FixedArray<ScenePos, 3> t{
            safe_stox<ScenePos>(match[3].str()),
            safe_stox<ScenePos>(match[4].str()),
            safe_stox<ScenePos>(match[5].str()) };
        FixedArray<float, 3> scale{
            safe_stof(match[6].str()),
            safe_stof(match[7].str()),
            safe_stof(match[8].str()) };
        float mean_scale = mean(scale);
        if (any(abs(scale - mean_scale) > 1e-3f)) {
            lwarn() << name << ": Scale is anisotropic: " << scale;
        }
        FixedArray<float, 3> v{
            safe_stof(match[9].str()),
            safe_stof(match[10].str()),
            safe_stof(match[11].str()) };
        float s = safe_stof(match[12].str());
        // if (auto l2 = sum(squared(v)) + squared(s);
        //     std::abs(l2 - 1.f) > 1e-6)
        // {
        //     lwarn() << "Normalizing quaternion for line \"" << line << '"';
        //     auto l = std::sqrt(l2);
        //     v /= l;
        //     s /= l;
        // }
        Quaternion<float> q{ -s, v };
        auto r = q.to_rotation_matrix();
        result.push_back(InstanceInformation{
            .resource_name = std::move(name),
            .trafo = { r, t },
            .scale = mean_scale,
            .rendering_dynamics = rendering_dynamics});
    }
    return result;
}
