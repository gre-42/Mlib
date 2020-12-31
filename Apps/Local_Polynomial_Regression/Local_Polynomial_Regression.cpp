#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Images/Filters/Gaussian_Filter.hpp>
#include <Mlib/Images/Filters/Local_Polynomial_Regression.hpp>
#include <Mlib/Images/Normalize.hpp>
#include <Mlib/Images/PgmImage.hpp>
#include <Mlib/String.hpp>
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
        local_polynomial_regression(bitmap.to_float(), [sigma, degree](const Array<float>& im){return gaussian_filter_NWE(im, sigma, NAN);}, degree),
        0.f,
        1.f));
    dest.save_to_file(destination);
}

int main(int argc, char **argv) {
    #ifndef __MINGW32__
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
