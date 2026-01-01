#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Images/Svg.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Physics/Collision/Pacejkas_Magic_Formula.hpp>
#include <Mlib/Stats/Linspace.hpp>
#include <fstream>

using namespace Mlib;

int main(int argc, char** argv) {
    const ArgParser parser(
        "Usage: plot_pacejkas_magic_formula --filename <filename> --n <n>",
        {},
        {"--filename", "--n"});
    try {
        const auto args = parser.parsed(argc, argv);
        args.assert_num_unnamed(0);
        auto destination = args.named_value("--filename");
        auto n = safe_stoz(args.named_value("--n"));
        float width = 800.f;
        float height = 600.f;
        std::ofstream ostr{ destination };
        if (ostr.fail()) {
            THROW_OR_ABORT("Could not open \"" + destination + "\" for write");
        }
        std::vector<std::vector<float>> X;
        std::vector<std::vector<float>> Y;
        X.reserve(n);
        Y.reserve(n);
        for (const auto& f : Linspace(-0.6f, 0.6f, n)) {
            PacejkasMagicFormula<float> mf{
                .B = 41.f * f,
                .C = 1.4f,
                .D = 1.f,
                .E = -0.2f,
            };
            auto x = linspace(-1.f, 1.f, 100) * 20.f / 180.f * (float)M_PI;
            X.push_back(x.to_vector());
            Y.push_back(x.applied(mf).to_vector());
        }
        Svg svg{ostr, width, height};
        svg.plot_multiple(X, Y);
        svg.finish();
        ostr.flush();
        if (ostr.fail()) {
            THROW_OR_ABORT("Could write to file \"" + destination + '"');
        }
    } catch (const std::exception& e) {
        lerr() << e.what();
        return 1;
    }
    return 0;
}
