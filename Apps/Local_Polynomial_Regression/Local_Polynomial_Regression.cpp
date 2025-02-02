#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Images/Filters/Gaussian_Filter.hpp>
#include <Mlib/Images/Filters/Local_Polynomial_Regression.hpp>
#include <Mlib/Images/Normalize.hpp>
#include <Mlib/Images/StbImage1.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <iostream>

using namespace Mlib;

void lpr(
    const std::string& source,
    const std::string& destination,
    double sigma,
    size_t degree,
    FilterExtension fc)
{
    auto bitmap = StbImage1::load_from_file(source);
    StbImage1 dest;
    dest = StbImage1::from_float_grayscale(clipped(
        local_polynomial_regression(
            bitmap.to_float_grayscale().casted<double>(),
            [sigma, fc](const Array<double>& im){
                return gaussian_filter_NWE<double>(im, sigma, NAN, 4, fc);
            },
            degree),
        0.0,
        1.0).casted<float>());
    dest.save_to_file(destination);
}

int main(int argc, char **argv) {
    enable_floating_point_exceptions();

    const ArgParser parser(
        "Usage: local_polynomial_regression source destination --sigma <sigma> --degree <degree> [--periodic]",
        {"--periodic"},
        {"--sigma", "--degree"});
    try {
        const auto args = parser.parsed(argc, argv);
        args.assert_num_unnamed(2);
        lpr(
            args.unnamed_value(0),
            args.unnamed_value(1),
            safe_stod(args.named_value("--sigma")),
            safe_stoz(args.named_value("--degree")),
            args.has_named("--periodic")
                ? FilterExtension::PERIODIC
                : FilterExtension::NONE);
    } catch (const std::runtime_error& e) {
        linfo() << e.what();
        return 1;
    }
    return 0;
}
