#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Geography/Heightmaps/Cities_Skylines.hpp>
#include <Mlib/Geography/Heightmaps/Terrarium.hpp>
#include <Mlib/Images/Bilinear_Interpolation.hpp>
#include <Mlib/Images/Draw_Bmp.hpp>
#include <Mlib/Images/Pgm_Image.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Stats/Min_Max.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <cpp-httplib/httplib.h>
#include <stb/stb_image_write.h>
#include <stb_cpp/stb_image_load.hpp>

using namespace Mlib;

StbInfo<uint8_t> download_tile(
    size_t tile_pixels,
    size_t zoom,
    size_t x,
    size_t y,
    const std::string& api_key,
    const std::string& filename)
{
    httplib::Client cli("https://tile.nextzen.org");
    std::stringstream sstr;
    // https://github.com/tangrams/heightmapper/blob/master/scene.yaml
    sstr << "/tilezen/terrain/v1/"
        << tile_pixels
        << "/terrarium/"
        << zoom
        << "/"
        << x
        << "/"
        << y
        << ".png?api_key="
        << api_key;
    lerr() << "Get: " << sstr.str();
    auto res = cli.Get(sstr.str().c_str());
    if (res->status != 200) {
        throw std::runtime_error("Error status: " + std::to_string(res->status) + "\n" + res->body);
    }
    if (filename.empty()) {
        std::vector<std::byte> data{
            (const std::byte*)res->body.data(),
            (const std::byte*)res->body.data() + res->body.length() };
        return stb_load8(filename, FlipMode::NONE, &data);
    } else {
        std::ofstream ofstr(filename, std::ios::binary);
        for (char c : res->body) {
            ofstr.put(c);
        }
        ofstr.flush();
        if (ofstr.fail()) {
            throw std::runtime_error("Could not write tile file \"" + filename + '"');
        }
        return stb_load8(filename, FlipMode::NONE);
    }
}

class CroppedTerrariumHeightmap {
public:
    CroppedTerrariumHeightmap(
        const Array<float>& map,
        float min_y_id,
        float max_y_id,
        float min_x_id,
        float max_x_id)
    : map_(map),
      min_y_id_{min_y_id},
      max_y_id_{max_y_id},
      min_x_id_{min_x_id},
      max_x_id_{max_x_id}
    {}
    bool operator () (float y, float x, float& intensity) const {
        float rf = min_y_id_ * (1 - y) + max_y_id_ * y;
        float cf = min_x_id_ * (1 - x) + max_x_id_ * x;
        return bilinear_grayscale_interpolation(rf, cf, map_, intensity);
    }
private:
    Array<float> map_;
    float min_y_id_;
    float max_y_id_;
    float min_x_id_;
    float max_x_id_;
};

/**
 * From: https://en.wikipedia.org/wiki/Mercator_projection#Derivation_of_the_Mercator_projection
 */
double get_y(double phi) {
    return std::log(std::tan(M_PI / 4. + phi / 2.));
}

int main(int argc, char** argv) {
    const char* help =
        "Usage: download_heightmap"
        " [--help]"
        " --zoom <zoom>"
        " --tile_pixels <tile_pixels>"
        " --result_width <result_width>"
        " --result_height <result_height>"
        " --min_lat <min_lat>"
        " --min_lon <min_lon>"
        " --max_lat <max_lat>"
        " --max_lon <max_lon>"
        " [--out_png <filename>]"
        " [--out_pgm <filename>]"
        " [--api_key <api_key>]"
        " [--tmp_png <filename>]"
        " [--stitched_png <filename>]"
        " [--stitched_normalized_png <filename>]"
        " [--resampled_normalized_png <filename>]";
    const ArgParser parser(
        help,
        {"--help"},
        {"--zoom",
         "--tile_pixels",
         "--result_width",
         "--result_height",
         "--min_lat",
         "--min_lon",
         "--max_lat",
         "--max_lon",
         "--api_key",
         "--tmp_filename",
         "--stitched_png",
         "--stitched_normalized_png",
         "--resampled_normalized_png",
         "--out_png",
         "--out_pgm",
         "--tmp_png"});
    try {
        const auto args = parser.parsed(argc, argv);
        if (args.has_named("--help")) {
            lout() << help;
            return 0;
        }
        args.assert_num_unnamed(0);
        size_t zoom = safe_stoz(args.named_value("--zoom"));
        double min_lat = safe_stod(args.named_value("--min_lat"));
        double min_lon = safe_stod(args.named_value("--min_lon"));
        double max_lat = safe_stod(args.named_value("--max_lat"));
        double max_lon = safe_stod(args.named_value("--max_lon"));
        size_t tile_pixels = safe_stoz(args.named_value("--tile_pixels"));
        size_t result_width = safe_stoz(args.named_value("--result_width"));
        size_t result_height = safe_stoz(args.named_value("--result_height"));
        std::string tmp_png = args.named_value("--tmp_png", "");

        // From: https://epsg.io/3857
        double max_y_global = M_PI;  // get_y(85.06 * degrees);
        double min_y_global = -M_PI; // get_y(-85.06 * degrees);
        size_t ntiles_global_y = (size_t)std::pow(2, zoom);
        size_t ntiles_global_x = (size_t)std::pow(2, zoom);
        double tile_len_y = (max_y_global - min_y_global) / (double)ntiles_global_y;
        double tile_len_x = 360. / (double)ntiles_global_x;
        double requested_min_y = get_y(-max_lat * degrees);
        double requested_max_y = get_y(-min_lat * degrees);
        size_t tiles_min_y = (size_t)std::floor((requested_min_y - min_y_global) / tile_len_y);
        size_t tiles_max_y = (size_t)std::ceil ((requested_max_y - min_y_global) / tile_len_y);
        size_t tiles_min_x = (size_t)std::floor((min_lon + 180) / tile_len_x);
        size_t tiles_max_x = (size_t)std::ceil ((max_lon + 180) / tile_len_x);
        size_t ntiles_y = (tiles_max_y - tiles_min_y) + 1;
        size_t ntiles_x = (tiles_max_x - tiles_min_x) + 1;
        lerr() << "ntiles_y " << ntiles_y;
        lerr() << "ntiles_x " << ntiles_x;
        lerr() <<
            tiles_min_x << '/' <<
            tiles_min_y << " - " <<
            tiles_max_x << '/' <<
            tiles_max_y;
        if (tiles_min_y >= ntiles_global_y) {
            throw std::runtime_error("min_y out of range");
        }
        if (tiles_max_y >= ntiles_global_y) {
            throw std::runtime_error("max_y out of range");
        }
        if (tiles_min_x >= ntiles_global_x) {
            throw std::runtime_error("min_x out of range");
        }
        if (tiles_max_x >= ntiles_global_x) {
            throw std::runtime_error("max_x out of range");
        }
        Array<float> stitched{ArrayShape{ntiles_y * tile_pixels, ntiles_x * tile_pixels}};
        std::vector<unsigned char> stitched_rgb(stitched.nelements() * 3);
        for (size_t a = 0; a < ntiles_y; ++a) {
            for (size_t o = 0; o < ntiles_x; ++o) {
                size_t y = tiles_min_y + a;
                size_t x = tiles_min_x + o;
                auto image = download_tile(
                    tile_pixels,
                    zoom,
                    x,
                    y,
                    args.named_value("--api_key", "LmmWmJx5QWGLTYXKJtAogg"),
                    tmp_png);
                if (image.nrChannels != 3 && image.nrChannels != 4) {
                    throw std::runtime_error("Only 3 or 4 channels are supported");
                }
                for (size_t da = 0; da < tile_pixels; ++da) {
                    for (size_t dl = 0; dl < tile_pixels; ++dl) {
                        unsigned char* rgb = &image[(da * (size_t)image.width  + dl) * (size_t)image.nrChannels];
                        size_t ga = da + a * tile_pixels;
                        size_t go = dl + o * tile_pixels;
                        // https://www.mapzen.com/blog/elevation/
                        stitched(ga, go) = terrarium_to_meters_pix<float>(rgb);
                        for (size_t c = 0; c < 3; ++c) {
                            stitched_rgb[(ga * stitched.shape(1) + go) * 3 + c] = rgb[c];
                        }
                    }
                }
            }
        }
        double min_y_actual = (double)tiles_min_y * tile_len_y + min_y_global;
        double max_y_actual = ((double)tiles_max_y + 1 - 1. / (double)tile_pixels) * tile_len_y + min_y_global;
        double min_lon_actual = (double)tiles_min_x * tile_len_x - 180.;
        double max_lon_actual = ((double)tiles_max_x + 1 - 1. / (double)tile_pixels) * tile_len_x - 180.;
        // double min_lat_actual = (tile_len_y * (2 * min_y - 1) - 180) / 2;
        // double max_lat_actual = (tile_len_y * (2 * max_y - 1) - 180) / 2;
        // double min_lon_actual = (tile_len_x * (2 * min_x - 1) - 360) / 2;
        // double max_lon_actual = (tile_len_x * (2 * max_x - 1) - 360) / 2;

        float min_y_id = float((requested_min_y - min_y_actual) / (max_y_actual - min_y_actual) * double(stitched.shape(0) - 1));
        float max_y_id = float((requested_max_y - min_y_actual) / (max_y_actual - min_y_actual) * double(stitched.shape(0) - 1));
        float min_x_id = float((min_lon - min_lon_actual) / (max_lon_actual - min_lon_actual) * double(stitched.shape(1) - 1));
        float max_x_id = float((max_lon - min_lon_actual) / (max_lon_actual - min_lon_actual) * double(stitched.shape(1) - 1));

        CroppedTerrariumHeightmap cth{
            stitched,
            min_y_id,
            max_y_id,
            min_x_id,
            max_x_id};
        Array<float> resampled{ArrayShape{result_height, result_width}};
        for (size_t r = 0; r < resampled.shape(0); ++r) {
            for (size_t c = 0; c < resampled.shape(1); ++c) {
                float intensity;
                if (cth(
                    float(r) / float(resampled.shape(0) - 1),
                    float(c) / float(resampled.shape(1) - 1),
                    intensity))
                {
                    resampled(r, c) = intensity;
                } else {
                    resampled(r, c) = NAN;
                }
            }
        }
        if (args.has_named_value("--stitched_png")) {
            if (any(stitched.shape() > INT_MAX)) {
                THROW_OR_ABORT("Stitched image too large");
            }
            if (!stbi_write_png(args.named_value("--stitched_png").c_str(), (int)stitched.shape(1), (int)stitched.shape(0), 3, stitched_rgb.data(), 0)) {
                throw std::runtime_error("Could not write \"" + args.named_value("--stitched_png") + '"');
            }
        }
        if (args.has_named_value("--stitched_normalized_png")) {
            draw_nan_masked_grayscale(stitched, 0, 0).save_to_file(args.named_value("--stitched_normalized_png"));
        }
        if (args.has_named_value("--resampled_normalized_png")) {
            draw_nan_masked_grayscale(resampled, 0, 0).save_to_file(args.named_value("--resampled_normalized_png"));
        }
        if (args.has_named_value("--out_png")) {
            Array<uint8_t> rgb = meters_to_terrarium(resampled);
            StbImage3::from_rgb(rgb).save_to_file(args.named_value("--out_png"));
        }
        if (args.has_named_value("--out_pgm")) {
            PgmImage{meters_to_cities_skylines(resampled)}.save_to_file(args.named_value("--out_pgm"));
        }
    } catch (const std::runtime_error& e) {
        lerr() << e.what();
        return 1;
    }
    return 0;
}
