#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Images/StbImage3.hpp>
#include <Mlib/Sfm/Components/Detect_Chessboard.hpp>
#include <Mlib/Strings/To_Number.hpp>

using namespace Mlib;
using namespace Mlib::Sfm;

int main(int argc, char** argv) {
    const ArgParser parser(
        "Usage: chessboard_detector source destination --nrows <nrows> --ncols <ncols>",
        {},
        {"--nrows", "--ncols"});
    const auto args = parser.parsed(argc, argv);
    args.assert_num_unnamed(2);
    const auto bitmap = StbImage3::load_from_file(args.unnamed_value(0));
    const Array<float> image = bitmap.to_float_grayscale();
    StbImage3 bmp;
    Array<FixedArray<float, 2>> p_x;
    Array<FixedArray<float, 2>> p_y;
    detect_chessboard(
        image,
        ArrayShape{
            safe_stoz(args.named_value("--nrows")),
            safe_stoz(args.named_value("--ncols"))},
        p_x,
        p_y,
        bmp);
    bmp.save_to_file(args.unnamed_value(1));
    lerr() << "Extrinsic grid points:\n" << Array<float>{p_x};
    lerr() << "Intrinsic grid points:\n" << Array<float>{p_y};
    return 0;
}
