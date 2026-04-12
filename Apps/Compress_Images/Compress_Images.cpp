#include <Mlib/Audio/Io/Mp3_Io.hpp>
#include <Mlib/Compression/Compress.hpp>
#include <Mlib/Images/Compression/Assemble_Tiles.hpp>
#include <Mlib/Images/Compression/Brightness_Image_Files.hpp>
#include <Mlib/Images/Compression/Crop_Image_File.hpp>
#include <Mlib/Images/Compression/Tile_Image_File.hpp>
#include <Mlib/Images/Filters/Lowpass_Filter_Extension.hpp>
#include <Mlib/Images/Target_Shape_Mode.hpp>
#include <Mlib/Images/Transform/Resize_File.hpp>
#include <Mlib/Io/Arg_Parser.hpp>
#include <Mlib/Json/Json_View.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Misc/FPath_Json.hpp>
#include <Mlib/Misc/Floating_Point_Exceptions.hpp>
#include <Mlib/Os/Pathes.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Strings/Utf8_Path.hpp>
#include <optional>
#include <regex>
#include <set>
#include <vector>

using namespace Mlib;

namespace LumaChrominanceArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(luma);
DECLARE_ARGUMENT(chrominance);
DECLARE_ARGUMENT(file_extension);
}

namespace FragmentArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(fragment_size);
DECLARE_ARGUMENT(assembled_size);
DECLARE_ARGUMENT(stepsize);
DECLARE_ARGUMENT(randsize);
DECLARE_ARGUMENT(channels);
DECLARE_ARGUMENT(alpha);
DECLARE_ARGUMENT(alpha_fac);
DECLARE_ARGUMENT(add);
DECLARE_ARGUMENT(upsampling);
DECLARE_ARGUMENT(ols);
}

namespace ResizeArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(size);
DECLARE_ARGUMENT(periodic);
DECLARE_ARGUMENT(jpg_quality);
DECLARE_ARGUMENT(file_extension);
}

namespace SedArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(substitute);
DECLARE_ARGUMENT(gzip);
}

namespace AudioArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(format);
DECLARE_ARGUMENT(bitrate);
}

namespace CompressionModesArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(path);
DECLARE_ARGUMENT(glob);
DECLARE_ARGUMENT(resize);
DECLARE_ARGUMENT(luma);
DECLARE_ARGUMENT(luma_chrominance);
DECLARE_ARGUMENT(fragment);
DECLARE_ARGUMENT(copy);
DECLARE_ARGUMENT(gzip);
DECLARE_ARGUMENT(sed);
DECLARE_ARGUMENT(audio);
}

namespace Mlib {

struct LumaChrominance {
    FixedArray<size_t, 2> luma = uninitialized;
    FixedArray<size_t, 2> chrominance = uninitialized;
    std::optional<Utf8Path> file_extension;
};

void from_json(const nlohmann::json& j, LumaChrominance& lc) {
    JsonView jv{j};
    jv.validate(LumaChrominanceArgs::options);
    lc.luma = jv.at<EFixedArray<size_t, 2>>(LumaChrominanceArgs::luma);
    lc.chrominance = jv.at<EFixedArray<size_t, 2>>(LumaChrominanceArgs::chrominance);
    lc.file_extension = jv.try_at<Utf8Path>(LumaChrominanceArgs::file_extension);
}

struct Fragment {
    FixedArray<uint32_t, 2> fragment_size = uninitialized;
    FixedArray<uint32_t, 2> assembled_size = uninitialized;
    float stepsize;
    float randsize;
    uint32_t channels;
    FPath alpha;
    FPath alpha_fac;
    bool add;
    uint32_t upsampling;
    bool ols;
};

void from_json(const nlohmann::json& j, Fragment& fragment) {
    JsonView jv{j};
    jv.validate(FragmentArgs::options);
    fragment.fragment_size = jv.at<EFixedArray<uint32_t, 2>>(FragmentArgs::fragment_size);
    fragment.assembled_size = jv.at<EFixedArray<uint32_t, 2>>(FragmentArgs::assembled_size);
    fragment.stepsize = jv.at<float>(FragmentArgs::stepsize);
    fragment.randsize = jv.at<float>(FragmentArgs::randsize);
    fragment.channels = jv.at<uint32_t>(FragmentArgs::channels);
    if (jv.contains_non_null(FragmentArgs::alpha)) {
        fragment.alpha = jv.at<FPath>(FragmentArgs::alpha);
    }
    if (jv.contains_non_null(FragmentArgs::alpha_fac)) {
        fragment.alpha_fac = jv.at<FPath>(FragmentArgs::alpha_fac);
    }
    fragment.add = jv.at<bool>(FragmentArgs::add);
    fragment.upsampling = jv.at<uint32_t>(FragmentArgs::upsampling);
    fragment.ols = jv.at<bool>(FragmentArgs::ols);
}

struct Resize {
    FixedArray<size_t, 2> size = uninitialized;
    bool periodic;
    int jpg_quality;
    std::optional<Utf8Path> file_extension;
};

void from_json(const nlohmann::json& j, Resize& resize) {
    JsonView jv{j};
    jv.validate(ResizeArgs::options);
    resize.size = jv.at<EFixedArray<size_t, 2>>(ResizeArgs::size);
    resize.periodic = jv.at<bool>(ResizeArgs::periodic);
    resize.jpg_quality = jv.at<int>(ResizeArgs::jpg_quality, 95);
    resize.file_extension = jv.try_at<Utf8Path>(ResizeArgs::file_extension);
    if ((resize.jpg_quality < 0) || (resize.jpg_quality > 100)) {
        throw std::runtime_error("JPG quality must be 0-100");
    }
}

using Substitute = std::vector<std::array<std::string, 2>>;

struct Sed {
    Substitute substitute;
    bool gzip;
};

void from_json(const nlohmann::json& j, Sed& sed) {
    JsonView jv{j};
    jv.validate(SedArgs::options);
    sed.substitute = jv.at<Substitute>(SedArgs::substitute);
    sed.gzip = jv.try_at<bool>(SedArgs::gzip).value_or(false);
}

using Glob = std::string;
using Copy = nlohmann::json;
using Gzip = nlohmann::json;
using LumaOnly = EFixedArray<size_t, 2>;

struct Audio {
    std::string format;
    uint32_t bitrate;
};

void from_json(const nlohmann::json& j, Audio& audio) {
    JsonView jv{j};
    jv.validate(AudioArgs::options);
    audio.format = jv.at<std::string>(AudioArgs::format);
    audio.bitrate = jv.at<uint32_t>(AudioArgs::bitrate);
}
struct CompressionModes {
    Utf8Path path;
    std::optional<Glob> glob;
    std::optional<Resize> resize;
    std::optional<LumaOnly> luma;
    std::optional<LumaChrominance> luma_chrominance;
    std::optional<Fragment> fragment;
    std::optional<Copy> copy;
    std::optional<Gzip> gzip;
    std::optional<Sed> sed;
    std::optional<Audio> audio;
};

void from_json(const nlohmann::json& j, CompressionModes& cm) {
    JsonView jv{j};
    jv.validate(CompressionModesArgs::options);
    cm.path = jv.at<Utf8Path>(CompressionModesArgs::path);
    cm.glob = jv.try_at<Glob>(CompressionModesArgs::glob);
    cm.resize = jv.try_at<Resize>(CompressionModesArgs::resize);
    cm.luma = jv.try_at<LumaOnly>(CompressionModesArgs::luma);
    cm.luma_chrominance = jv.try_at<LumaChrominance>(CompressionModesArgs::luma_chrominance);
    cm.fragment = jv.try_at<Fragment>(CompressionModesArgs::fragment);
    cm.copy = jv.try_at<Copy>(CompressionModesArgs::copy);
    cm.gzip = jv.try_at<Gzip>(CompressionModesArgs::gzip);
    cm.sed = jv.try_at<Sed>(CompressionModesArgs::sed);
    cm.audio = jv.try_at<Audio>(CompressionModesArgs::audio);
}

}

static bool file_missing_or_outdated(
    const Utf8Path& source,
    const Utf8Path& target)
{
    if (!path_exists(target)) {
        return true;
    }
    if (std::filesystem::last_write_time(source) > std::filesystem::last_write_time(target)) {
        return true;
    }
    return false;
}

void sed(
    const Utf8Path& source,
    const Utf8Path& destination,
    const Sed& sed)
{
    auto fi = create_ifstream(source, std::ios::binary);
    if (fi->fail()) {
        throw std::runtime_error("Could not open for read: \"" + source.string() + '"');
    }
    auto s = std::string(std::istreambuf_iterator(*fi), {});
    if (fi->fail()) {
        throw std::runtime_error("Could not read from: \"" + source.string() + '"');
    }
    for (const auto& sub : sed.substitute) {
        s = std::regex_replace(s, std::regex(sub[0]), sub[1]);
    }
    auto fo = create_ofstream(destination, std::ios::binary);
    if (fo->fail()) {
        throw std::runtime_error("Could not open for write: \"" + destination.string() + '"');
    }
    if (sed.gzip) {
        *fo << compressed_string(s);
    } else {
        *fo << s;
    }
    fo->flush();
    if (fo->fail()) {
        throw std::runtime_error("Could not write to file: \"" + destination.string() + '"');
    }
}

void convert(
    const Utf8Path& source_dir,
    const Utf8Path& dest_dir,
    const Utf8Path& source_name,
    const CompressionModes& cm,
    const ParsedArgs& args)
{
    bool overwrite_existing = args.has_named("--overwrite_existing");
    auto rel_path = Utf8Path{source_name};
    auto dest_parent = dest_dir / rel_path.parent_path();
    auto stem = rel_path.stem();
    auto extension = rel_path.extension();
    auto derived_dest_path = [&](const std::string& suffix){
        return dest_parent / (stem.string() + '.' + suffix + extension.string());
    };
    auto derived_dest_extension = [&](const std::string& extension){
        return dest_parent / (stem.string() + extension);
    };
    auto verbose_update_intended = [&](const Utf8Path& dest){
        if (overwrite_existing || file_missing_or_outdated(source_dir / rel_path, dest)) {
            linfo() << "Updating " << source_dir / rel_path << " -> " << dest;
            return true;
        }
        return false;
    };
    if (cm.resize.has_value()) {
        auto resized_path = dest_dir / rel_path;
        if (cm.resize->file_extension.has_value()) {
            resized_path.replace_extension(*cm.resize->file_extension);
        }
        if (overwrite_existing || verbose_update_intended(resized_path)) {
            std::filesystem::create_directories(dest_parent);
            auto filter_extension = cm.resize->periodic
                ? FilterExtension::PERIODIC
                : FilterExtension::NWE;
            resize_file(
                source_dir / rel_path,
                resized_path,
                cm.resize->size,
                filter_extension,
                TargetShapeMode::SOURCE_WHEN_ZERO,
                cm.resize->jpg_quality);
        }
    }
    if (cm.luma.has_value()) {
        auto luma_path = dest_dir / rel_path;
        if (overwrite_existing || verbose_update_intended(luma_path)) {
            BrightnessImageFiles bi{
                source_dir / rel_path,
                1,
                1,
                (*cm.luma)(0),
                (*cm.luma)(1),
                TargetShapeMode::SOURCE_WHEN_ZERO};
            std::filesystem::create_directories(dest_parent);
            bi.save_brightness_and_alpha(luma_path);
        }
    }
    if (cm.luma_chrominance.has_value()) {
        auto luma_path = derived_dest_path("luma");
        auto chrominance_path = derived_dest_path("chrominance");
        if (cm.luma_chrominance->file_extension.has_value()) {
            luma_path.replace_extension(*cm.luma_chrominance->file_extension);
            chrominance_path.replace_extension(*cm.luma_chrominance->file_extension);
        }
        if (overwrite_existing || verbose_update_intended(luma_path)) {
            BrightnessImageFiles bi{
                source_dir / rel_path,
                cm.luma_chrominance->chrominance(0),
                cm.luma_chrominance->chrominance(1),
                cm.luma_chrominance->luma(0),
                cm.luma_chrominance->luma(1),
                TargetShapeMode::SOURCE_WHEN_ZERO};
            std::filesystem::create_directories(dest_parent);
            bi.save_brightness_and_alpha(luma_path);
            bi.save_color(chrominance_path);
            if (args.has_named("--save_reconstructed")) {
                bi.save_reconstructed(derived_dest_path("recon"));
            }
        }
    }
    if (cm.fragment.has_value()) {
        auto dest_path = derived_dest_extension(".tiles.json");
        if (overwrite_existing || verbose_update_intended(dest_path)) {
            auto dest_fragment_path = derived_dest_path("fragment");
            std::filesystem::create_directories(dest_parent);
            auto fragment_size_z = FixedArray<size_t, 2>{
                integral_cast<size_t>(cm.fragment->fragment_size(0)),
                integral_cast<size_t>(cm.fragment->fragment_size(1))
            };
            crop_image_file(source_dir / rel_path, dest_fragment_path, fragment_size_z);
            auto fa = FragmentAssembly{
                .color = FPath::from_local_path(dest_fragment_path.lexically_relative(dest_parent)),
                .alpha = cm.fragment->alpha,
                .alpha_fac = cm.fragment->alpha_fac,
                .size = cm.fragment->assembled_size,
                .stepsize = cm.fragment->stepsize,
                .randsize = cm.fragment->randsize,
                .channels = cm.fragment->channels,
                .add = cm.fragment->add,
                .upsampling = cm.fragment->upsampling};
            if (cm.fragment->ols) {
                auto tmp = fa;
                tmp.ols.emplace();
                tmp.make_pathes_absolute(dest_path);
                assemble_tiles_compute_ols(tmp);
                fa.ols = tmp.ols;
            }
            save_fragment_assembly(dest_path, fa);
        }
    }
    if (cm.copy.has_value()) {
        auto dest_path = dest_dir / rel_path;
        if (overwrite_existing || verbose_update_intended(dest_path)) {
            std::filesystem::create_directories(dest_parent);
            std::filesystem::copy_file(source_dir / rel_path, dest_path, std::filesystem::copy_options::overwrite_existing);
        }
    }
    if (cm.gzip.has_value()) {
        auto dest_path = dest_dir / (rel_path.string() + ".gz");
        if (overwrite_existing || verbose_update_intended(dest_path)) {
            std::filesystem::create_directories(dest_parent);
            compress_file(source_dir / rel_path, dest_path);
        }
    }
    if (cm.sed.has_value()) {
        auto dest_path = cm.sed->gzip
            ? dest_dir / (rel_path.string() + ".gz")
            : dest_dir / rel_path;
        if (overwrite_existing || verbose_update_intended(dest_path)) {
            std::filesystem::create_directories(dest_parent);
            sed(source_dir / rel_path, dest_path, *cm.sed);
        }
    }
    if (cm.audio.has_value()) {
        if (cm.audio->format == "MP3") {
            auto mp3 = Utf8Path{rel_path}.replace_extension(".mp3");
            auto dest_path = dest_dir / mp3;
            if (overwrite_existing || verbose_update_intended(dest_path)) {
                std::filesystem::create_directories(dest_parent);
                convert_to_mp3(source_dir / rel_path, dest_path, cm.audio->bitrate);
            }
        } else {
            throw std::runtime_error("Unknown audio format: \"" + cm.audio->format + '"');
        }
    }
}

int main(int argc, char** argv) {
    enable_floating_point_exceptions();

    const char* help =
        "Usage: compress_images "
        "[--help] "
        "--source_dirs <path1;path2> "
        "--configs path1=compression1.json;path2=compression2.json;... "
        "[--save_reconstructed] "
        "[--overwrite_existing]";
    const ArgParser parser(
        help,
        {"--help",
         "--save_reconstructed",
         "--overwrite_existing"},
        {"--source_dirs",
         "--configs"});
    try {
        const auto args = parser.parsed(argc, argv);
        if (args.has_named("--help")) {
            lout() << help;
            return 0;
        }
        args.assert_num_unnamed(0);
        nlohmann::json j;
        auto configs = split_semicolon_separated_pairs_of_pathes(args.named_value("--configs", "compression.json"));
        for (const auto& [dest_dir, c] : configs) {
            auto f = create_ifstream(c);
            if (f->fail()) {
                throw std::runtime_error("Could not open \"" + c.string() + "\" for reading");
            }
            *f >> j;
            if (f->fail()) {
                throw std::runtime_error("Could not read from \"" + c.string() + "\"");
            }
            auto source_dirs = split_semicolon_separated_pathes(args.named_value("--source_dirs"));
            for (const auto& cm : j.get<std::vector<CompressionModes>>()) {
                if (cm.glob.has_value()) {
                    std::set<Utf8Path> rel_source_names;
                    auto re = std::regex{*cm.glob};
                    for (const auto& source_dir : source_dirs) {
                        if (!path_exists(source_dir / cm.path)) {
                            continue;
                        }
                        for (const auto& source_name : list_dir_recursive(source_dir / cm.path)) {
                            if (source_name.is_directory()) {
                                continue;
                            }
                            auto rel_source_name = Utf8Path::from_path(source_name.path().lexically_relative(source_dir));
                            if (!std::regex_search(rel_source_name.string(), re)) {
                                continue;
                            }
                            if (rel_source_names.contains(rel_source_name)) {
                                throw std::runtime_error("Found multiple files: \"" + rel_source_name.string() + '"');
                            }
                            try {
                                convert(source_dir, dest_dir, rel_source_name, cm, args);
                            } catch (const std::runtime_error& e) {
                                throw std::runtime_error("Error processing file \"" + rel_source_name.string() + "\": " + e.what());
                            }
                            rel_source_names.emplace(std::move(rel_source_name));
                        }
                    }
                    if (rel_source_names.empty()) {
                        throw std::runtime_error("Found no file: \"" + cm.path.string() + '"');
                    }
                } else {
                    size_t nfound = 0;
                    for (const auto& source_dir : source_dirs) {
                        if (!path_exists(source_dir / cm.path)) {
                            continue;
                        }
                        if (nfound++ != 0) {
                            throw std::runtime_error("Found multiple files: \"" + cm.path.string() + '"');
                        }
                        try {
                            convert(source_dir, dest_dir, cm.path, cm, args);
                        } catch (const std::runtime_error& e) {
                            throw std::runtime_error("Error processing file \"" + cm.path.string() + "\": " + e.what());
                        }
                    }
                    if (nfound != 1) {
                        throw std::runtime_error("Found no file: \"" + cm.path.string() + '"');
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        lerr() << e.what();
        return 1;
    }
    return 0;
}
