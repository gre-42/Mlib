#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Images/PgmImage.hpp>
#include <Mlib/String.hpp>
#include <cpp-httplib/httplib.h>
#include <stb_image/stb_image_load.h>
#include <stb_image/stb_image_write.h>

using namespace Mlib;

void download_tile(size_t tile_pixels, size_t zoom, size_t x, size_t y)
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
        << ".png?api_key=dmlO1fVQRPKI-GrVIYJ1YA";
    std::cerr << "Get: " << sstr.str() << std::endl;
    auto res = cli.Get(sstr.str().c_str());
    if (res->status != 200) {
        throw std::runtime_error("Error status: " + std::to_string(res->status));
    }
    std::ofstream ofstr("/tmp/tile.png", std::ios::binary);
    for(char c : res->body) {
        ofstr.put(c);
    }
    ofstr.flush();
    if (ofstr.fail()) {
        throw std::runtime_error("Could not write tile file");
    }
}

int main(int argc, char** argv) {
    const ArgParser parser(
        "Usage: download_heightmap"
        " --zoom <zoom>"
        " --tile_pixels <tile_pixels>"
        " --min_lat <min_lat>"
        " --min_lon <min_lon>"
        " --max_lat <max_lat>"
        " --max_lon <max_lon>",
        {},
        {"--zoom", "--tile_pixels", "--min_lat", "--min_lon", "--max_lat", "--max_lon"});
    const auto args = parser.parsed(argc, argv);
    args.assert_num_unamed(0);
    float zoom = safe_stof(args.named_value("--zoom"));
    float min_lat = safe_stof(args.named_value("--min_lat"));
    float min_lon = safe_stof(args.named_value("--min_lon"));
    float max_lat = safe_stof(args.named_value("--max_lat"));
    float max_lon = safe_stof(args.named_value("--max_lon"));
    size_t tile_pixels = safe_stoz(args.named_value("--tile_pixels"));

    size_t ntiles_global = std::pow(2, safe_stoz(args.named_value("--zoom")));
    float tile_len = 180.f / ntiles_global;
    size_t ntiles_lat = std::ceil((max_lat - min_lat) / tile_len);
    size_t ntiles_lon = std::ceil((max_lon - min_lon) / tile_len);
    PgmImage res{ArrayShape{ntiles_lat * tile_pixels, ntiles_lon * tile_pixels}};
    std::vector<unsigned char> res_rgb(res.nelements() * 3);
    std::cerr << "ntiles_lat " << ntiles_lat << std::endl;
    std::cerr << "ntiles_lon " << ntiles_lon << std::endl;
    for(size_t a = 0; a < ntiles_lat; ++a) {
        for(size_t o = 0; o < ntiles_lon; ++o) {
            float lat = min_lat + a * tile_len;
            float lon = min_lon + o * tile_len;
            download_tile(
                tile_pixels,
                zoom,
                (lon + tile_len / 2 + 180) / 360 * 1024,
                (lat + tile_len / 2 + 180) / 360 * 1024);
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
                    res(ga, go) =
                        ((rgb[0] * 256 * 256 + 256 * rgb[1] + rgb[2]) - 32768) / 2;
                    for (size_t c = 0; c < 3; ++c) {
                        res_rgb[(ga * res.shape(1)  + go) * 3 + c] = rgb[c];
                    }
                }
            }
        }
    }
    res.save_to_file("/tmp/heightmap.pgm");
    stbi_write_png("/tmp/heightmap.png", res.shape(1), res.shape(0), 3, res_rgb.data(), 0);
    
    return 0;
}
