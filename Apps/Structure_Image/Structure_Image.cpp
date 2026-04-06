#include <Mlib/Images/Compression/Brightness_Image_Files.hpp>
#include <Mlib/Images/Target_Shape_Mode.hpp>
#include <Mlib/Io/Arg_Parser.hpp>
#include <Mlib/Misc/Floating_Point_Exceptions.hpp>

using namespace Mlib;

int main(int argc, char** argv) {
    enable_floating_point_exceptions();

    const char* help =
        "Usage: structure_image "
        "[--help] "
        "--source <file> "
        "--dest_color <filename> "        
        "--dest_structure <filename> "
        "[--dest_reconstructed <filename> ]"
        "--color_width <w> "
        "--color_height <h> "
        "--structure_width <w> "
        "--structure_height <h>";
    const ArgParser parser(
        help,
        {"--help"},
        {"--source",
         "--dest_color",
         "--dest_structure",
         "--dest_reconstructed",
         "--color_width",
         "--color_height",
         "--structure_width",
         "--structure_height"});
    try {
        const auto args = parser.parsed(argc, argv);
        if (args.has_named("--help")) {
            lout() << help;
            return 0;
        }
        args.assert_num_unnamed(0);
        BrightnessImageFiles bi{
            args.named_value("--source"),
            safe_stoz(args.named_value("--color_width")),
            safe_stoz(args.named_value("--color_height")),
            safe_stoz(args.named_value("--structure_width")),
            safe_stoz(args.named_value("--structure_height")),
            TargetShapeMode::SOURCE_WHEN_ZERO};
        if (auto dest_color = args.try_named_value("--dest_color"); dest_color != nullptr) {
            bi.save_color(*dest_color);
        }
        if (auto dest_structure = args.try_named_value("--dest_structure"); dest_structure != nullptr) {
            bi.save_brightness_and_alpha(*dest_structure);
        }
        if (auto dest_reconstructed = args.try_named_value("--dest_reconstructed"); dest_reconstructed != nullptr) {
            bi.save_reconstructed(*dest_reconstructed);
        }
    } catch (const std::exception& e) {
        lerr() << e.what();
        return 1;
    }
    return 0;
}
