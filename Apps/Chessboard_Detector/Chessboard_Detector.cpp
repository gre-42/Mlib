#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Images/PpmImage.hpp>
#include <Mlib/Sfm/Components/Detect_Chessboard.hpp>
#include <Mlib/String.hpp>

using namespace Mlib;
using namespace Mlib::Sfm;

int main(int argc, char** argv) {
    const ArgParser parser(
        "Usage: chessboard_detector source destination --nrows <nrows> --ncols <ncols>",
        {},
        {"--nrows", "--ncols"});
    const auto args = parser.parsed(argc, argv);
    args.assert_num_unamed(2);
    const auto bitmap = PpmImage::load_from_file(args.unnamed_value(0));
    const Array<float> image = bitmap.to_float_grayscale();
    PpmImage bmp;
    Array<float> p_x;
    Array<float> p_y;
    detect_chessboard(
        image,
        ArrayShape{
            (size_t)safe_stoi(args.named_value("--nrows")),
            (size_t)safe_stoi(args.named_value("--ncols"))},
        p_x,
        p_y,
        bmp);
    bmp.save_to_file(args.unnamed_value(1));
    std::cerr << "Extrinsic grid points:\n" << p_x;
    std::cerr << "Intrinsic grid points:\n" << p_y;
    return 0;
}
