#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Images/Bilinear_Interpolation.hpp>
#include <Mlib/Images/Draw_Bmp.hpp>
#include <Mlib/Images/PgmImage.hpp>
#include <Mlib/Stats/Min_Max.hpp>
#include <Mlib/String.hpp>
#include <cpp-httplib/httplib.h>
#include <stb_image/stb_image_load.h>
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
    for(char c : res->body) {
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
        Array<float> map,
        float min_y_id,
        float max_y_id,
        float min_x_id,
        float max_x_id)
    : map_{map},
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

int main(int argc, char** argv) {
    const ArgParser parser(
        "Usage: download_heightmap"
        " --zoom <zoom>"
        " --tile_pixels <tile_pixels>"
        " --result_pixels <result_pixels>"
        " --min_lat <min_lat>"
        " --min_lon <min_lon>"
        " --max_lat <max_lat>"
        " --max_lon <max_lon>"
        " --out_pgm <filename>"
        " [--api_key <api_key>]"
        " [--tmp_filename <filename>]"
        " [--stitched_png <filename>]"
        " [--stitched_pgm <filename>]"
        " [--resampled_pgm <filename>]",
        {},
        {"--zoom",
         "--tile_pixels",
         "--result_pixels",
         "--min_lat",
         "--min_lon",
         "--max_lat",
         "--max_lon",
         "--api_key",
         "--tmp_filename",
         "--tmp_filename",
         "--stitched_png",
         "--stitched_pgm",
         "--resampled_pgm",
         "--out_pgm"});
    const auto args = parser.parsed(argc, argv);
    args.assert_num_unamed(0);
    float zoom = safe_stof(args.named_value("--zoom"));
    float min_lat = safe_stof(args.named_value("--min_lat"));
    float min_lon = safe_stof(args.named_value("--min_lon"));
    float max_lat = safe_stof(args.named_value("--max_lat"));
    float max_lon = safe_stof(args.named_value("--max_lon"));
    size_t tile_pixels = safe_stoz(args.named_value("--tile_pixels"));
    size_t result_pixels = safe_stoz(args.named_value("--result_pixels"));
    std::string out_pgm = args.named_value("--out_pgm");

    size_t ntiles_global_y = std::pow(2, safe_stoz(args.named_value("--zoom"))) / 2;
    size_t ntiles_global_x = std::pow(2, safe_stoz(args.named_value("--zoom")));
    float tile_len_y = 180.f / ntiles_global_y;
    float tile_len_x = 360.f / ntiles_global_x;
    size_t min_y = std::floor((min_lat + tile_len_y / 2 + 90) / tile_len_y);
    size_t max_y = std::ceil ((max_lat - tile_len_y / 2 + 90) / tile_len_y);
    size_t min_x = std::floor((min_lon + tile_len_x / 2 + 180) / tile_len_x);
    size_t max_x = std::ceil ((max_lon - tile_len_x / 2 + 180) / tile_len_x);
    size_t ntiles_y = (max_y - min_y) + 1;
    size_t ntiles_x = (max_x - min_x) + 1;
    std::cerr << "ntiles_y " << ntiles_y << std::endl;
    std::cerr << "ntiles_x " << ntiles_x << std::endl;
    Array<float> res{ArrayShape{ntiles_y * tile_pixels, ntiles_x * tile_pixels}};
    std::vector<unsigned char> res_rgb(res.nelements() * 3);
    for(size_t a = 0; a < ntiles_y; ++a) {
        for(size_t o = 0; o < ntiles_x; ++o) {
            size_t y = min_y + a;
            size_t x = min_x + o;
            download_tile(
                tile_pixels,
                zoom,
                x,
                y,
                args.named_value("--api_key", "LmmWmJx5QWGLTYXKJtAogg"),
                args.named_value("--tmp_filename", "/tmp/tile.png"));
            StbInfo image = stb_load("/tmp/tile.png", false, false);
            if (image.nrChannels != 3 && image.nrChannels != 4) {
                throw std::runtime_error("Only 3 or 4 channels are supported");
            }
            for (size_t da = 0; da < tile_pixels; ++da) {
                for (size_t dl = 0; dl < tile_pixels; ++dl) {
                    unsigned char* rgb = &image.data.get()[(da * image.width  + dl) * image.nrChannels];
                    size_t ga = da + a * tile_pixels;
                    size_t go = dl + o * tile_pixels;
                    // https://www.mapzen.com/blog/elevation/
                    res(ga, go) = ((rgb[0] * 256.f + rgb[1] + rgb[2] / 256.f) - 32768.f) / 2.f;
                    for (size_t c = 0; c < 3; ++c) {
                        res_rgb[(ga * res.shape(1)  + go) * 3 + c] = rgb[c];
                    }
                }
            }
        }
    }
    float min_lat_actual = min_y * tile_len_y - tile_len_y / 2 - 90;
    float max_lat_actual = max_y * tile_len_y + tile_len_y / 2 - 90;
    float min_lon_actual = min_x * tile_len_x - tile_len_x / 2 - 180;
    float max_lon_actual = max_x * tile_len_x + tile_len_x / 2 - 180;
    // float min_lat_actual = (tile_len_y * (2 * min_y - 1) - 180) / 2;
    // float max_lat_actual = (tile_len_y * (2 * max_y - 1) - 180) / 2;
    // float min_lon_actual = (tile_len_x * (2 * min_x - 1) - 360) / 2;
    // float max_lon_actual = (tile_len_x * (2 * max_x - 1) - 360) / 2;

    float min_y_id = (min_lat - min_lat_actual) / (max_lat_actual - min_lat_actual) * (res.shape(0) - 1);
    float max_y_id = (max_lat - min_lat_actual) / (max_lat_actual - min_lat_actual) * (res.shape(0) - 1);
    float min_x_id = (min_lon - min_lon_actual) / (max_lon_actual - min_lon_actual) * (res.shape(1) - 1);
    float max_x_id = (max_lon - min_lon_actual) / (max_lon_actual - min_lon_actual) * (res.shape(1) - 1);

    CroppedTerrariumHeightmap cth{
        res,
        min_y_id,
        max_y_id,
        min_x_id,
        max_x_id};
    Array<float> resampled{ArrayShape{result_pixels, result_pixels}};
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
        stbi_write_png(args.named_value("--stitched_png").c_str(), res.shape(1), res.shape(0), 3, res_rgb.data(), 0);
    }
    if (args.has_named_value("--stitched_pgm")) {
        draw_nan_masked_grayscale(res, 0, 0).save_to_file(args.named_value("--stitched_gpm"));
    }
    if (args.has_named_value("--resampled_pgm")) {
        draw_nan_masked_grayscale(resampled, 0, 0).save_to_file(args.named_value("--resampled_pgm"));
    }
    PgmImage::from_float((resampled - min(resampled)) * 64.f / float(UINT16_MAX))
    .save_to_file(out_pgm);
    
    return 0;
}
