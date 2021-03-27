#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Images/Filters/Gaussian_Filter.hpp>
#include <Mlib/Images/Filters/Local_Polynomial_Regression.hpp>
#include <Mlib/Images/Normalize.hpp>
#include <Mlib/Images/PgmImage.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <fenv.h>
#include <iostream>

using namespace Mlib;

void lpr(
    const std::string& source,
    const std::string& destination,
    float sigma,
    size_t degree)
{
    auto bitmap = PgmImage::load_from_file(source);
    PgmImage dest;
    dest = PgmImage::from_float(clipped(
        local_polynomial_regression(
            bitmap.to_float().casted<double>(),
            [sigma, degree](const Array<double>& im){
                return gaussian_filter_NWE<double>(im, sigma, NAN, 4, false);
            },
            degree),
        0.0,
        1.0).casted<float>());
    dest.save_to_file(destination);
}

#ifdef _MSC_VER
#pragma float_control(except, on)
#endif

int main(int argc, char **argv) {
    #ifdef __linux__
    feenableexcept(FE_INVALID);
    #endif

    const ArgParser parser(
        "Usage: local_polynomial_regression source destination --sigma <sigma> --degree <degree>",
        {},
        {"--sigma", "--degree"});
    const auto args = parser.parsed(argc, argv);
    args.assert_num_unamed(2);
    lpr(
        args.unnamed_value(0),
        args.unnamed_value(1),
        safe_stof(args.named_value("--sigma")),
        safe_stoz(args.named_value("--degree")));
    return 0;
}
