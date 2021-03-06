#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Images/Bilinear_Interpolation.hpp>
#include <Mlib/Images/Draw_Bmp.hpp>
#include <Mlib/Images/Normalize.hpp>
#include <Mlib/Images/PgmImage.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Stats/Min_Max.hpp>
#include <Mlib/Strings/From_Number.hpp>
#include <cpp-httplib/httplib.h>
#include <stb_image/stb_image_load.hpp>
#include <stb_image/stb_image_write.h>

using namespace Mlib;

void download_tile(
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
    std::cerr << "Get: " << sstr.str() << std::endl;
    auto res = cli.Get(sstr.str().c_str());
    if (res->status != 200) {
        throw std::runtime_error("Error status: " + std::to_string(res->status) + "\n" + res->body);
    }
    std::ofstream ofstr(filename, std::ios::binary);
    for (char c : res->body) {
        ofstr.put(c);
    }
    ofstr.flush();
    if (ofstr.fail()) {
        throw std::runtime_error("Could not write tile file \"" + filename + '"');
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
float get_y(float phi) {
    return std::log(std::tan(M_PI / 4 + phi / 2));
}

int main(int argc, char** argv) {
    const ArgParser parser(
        "Usage: download_heightmap"
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
        " [--resampled_normalized_png <filename>]",
        {},
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
         "--out_pgm"});
    try {
        const auto args = parser.parsed(argc, argv);
        args.assert_num_unamed(0);
        size_t zoom = safe_stoz(args.named_value("--zoom"));
        float min_lat = safe_stof(args.named_value("--min_lat"));
        float min_lon = safe_stof(args.named_value("--min_lon"));
        float max_lat = safe_stof(args.named_value("--max_lat"));
        float max_lon = safe_stof(args.named_value("--max_lon"));
        size_t tile_pixels = safe_stoz(args.named_value("--tile_pixels"));
        size_t result_width = safe_stoz(args.named_value("--result_width"));
        size_t result_height = safe_stoz(args.named_value("--result_height"));
        std::string tmp_png = args.named_value("--tmp_png", "/tmp/tile.png");

        // From: https://epsg.io/3857
        float max_y_global = M_PI;  // get_y(85.06 * degrees);
        float min_y_global = -M_PI; // get_y(-85.06 * degrees);
        size_t ntiles_global_y = std::pow(2, zoom);
        size_t ntiles_global_x = std::pow(2, zoom);
        float tile_len_y = (max_y_global - min_y_global) / ntiles_global_y;
        float tile_len_x = 360.f / ntiles_global_x;
        float requested_min_y = get_y(-max_lat * degrees);
        float requested_max_y = get_y(-min_lat * degrees);
        size_t tiles_min_y = std::floor((requested_min_y - min_y_global) / tile_len_y);
        size_t tiles_max_y = std::ceil ((requested_max_y - min_y_global) / tile_len_y);
        size_t tiles_min_x = std::floor((min_lon + 180) / tile_len_x);
        size_t tiles_max_x = std::ceil ((max_lon + 180) / tile_len_x);
        size_t ntiles_y = (tiles_max_y - tiles_min_y) + 1;
        size_t ntiles_x = (tiles_max_x - tiles_min_x) + 1;
        std::cerr << "ntiles_y " << ntiles_y << std::endl;
        std::cerr << "ntiles_x " << ntiles_x << std::endl;
        std::cerr <<
            tiles_min_x << '/' <<
            tiles_min_y << " - " <<
            tiles_max_x << '/' <<
            tiles_max_y << std::endl;
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
                download_tile(
                    tile_pixels,
                    zoom,
                    x,
                    y,
                    args.named_value("--api_key", "LmmWmJx5QWGLTYXKJtAogg"),
                    tmp_png);
                StbInfo image = stb_load(tmp_png, false, false);
                if (image.nrChannels != 3 && image.nrChannels != 4) {
                    throw std::runtime_error("Only 3 or 4 channels are supported");
                }
                for (size_t da = 0; da < tile_pixels; ++da) {
                    for (size_t dl = 0; dl < tile_pixels; ++dl) {
                        unsigned char* rgb = &image.data.get()[(da * image.width  + dl) * image.nrChannels];
                        size_t ga = da + a * tile_pixels;
                        size_t go = dl + o * tile_pixels;
                        // https://www.mapzen.com/blog/elevation/
                        stitched(ga, go) = (rgb[0] * 256.f + rgb[1] + rgb[2] / 256.f) - 32768.f;
                        for (size_t c = 0; c < 3; ++c) {
                            stitched_rgb[(ga * stitched.shape(1) + go) * 3 + c] = rgb[c];
                        }
                    }
                }
            }
        }
        float min_y_actual = tiles_min_y * tile_len_y + min_y_global;
        float max_y_actual = (tiles_max_y + 1 - 1.f / tile_pixels) * tile_len_y + min_y_global;
        float min_lon_actual = tiles_min_x * tile_len_x - 180;
        float max_lon_actual = (tiles_max_x + 1 - 1.f / tile_pixels) * tile_len_x - 180;
        // float min_lat_actual = (tile_len_y * (2 * min_y - 1) - 180) / 2;
        // float max_lat_actual = (tile_len_y * (2 * max_y - 1) - 180) / 2;
        // float min_lon_actual = (tile_len_x * (2 * min_x - 1) - 360) / 2;
        // float max_lon_actual = (tile_len_x * (2 * max_x - 1) - 360) / 2;

        float min_y_id = (requested_min_y - min_y_actual) / (max_y_actual - min_y_actual) * (stitched.shape(0) - 1);
        float max_y_id = (requested_max_y - min_y_actual) / (max_y_actual - min_y_actual) * (stitched.shape(0) - 1);
        float min_x_id = (min_lon - min_lon_actual) / (max_lon_actual - min_lon_actual) * (stitched.shape(1) - 1);
        float max_x_id = (max_lon - min_lon_actual) / (max_lon_actual - min_lon_actual) * (stitched.shape(1) - 1);

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
                    r / float(resampled.shape(0) - 1),
                    c / float(resampled.shape(1) - 1),
                    intensity))
                {
                    resampled(r, c) = intensity;
                } else {
                    resampled(r, c) = NAN;
                }
            }
        }
        if (args.has_named_value("--stitched_png")) {
            if (!stbi_write_png(args.named_value("--stitched_png").c_str(), stitched.shape(1), stitched.shape(0), 3, stitched_rgb.data(), 0)) {
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
            Array<float> rgb{ ArrayShape{3, resampled.shape(0), resampled.shape(1) }};
            // https://www.mapzen.com/blog/elevation/
            Array<float> val = resampled + 32768.f;
            for (size_t c = 2; c != SIZE_MAX; --c) {
                rgb[c] = val - val.applied([](float v){return std::floor(v);});
                val -= rgb[c];
                val /= 256.f;
            }
            StbImage::from_float_rgb(rgb).save_to_file(args.named_value("--out_png"));
        }
        if (args.has_named_value("--out_pgm")) {
            PgmImage::from_float(
                clipped(
                    resampled * 64.f / float(UINT16_MAX),
                    0.f,
                    1.f)).save_to_file(args.named_value("--out_pgm"));
        }
        
        return 0;
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
    }
}
