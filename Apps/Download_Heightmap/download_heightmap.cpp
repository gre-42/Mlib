#include <Mlib/Arg_Parser.hpp>
#include <Mlib/String.hpp>
#include <cpp-httplib/httplib.h>
#include <stb_image/stb_image_load.h>

using namespace Mlib;

int main(int argc, char** argv) {
    const ArgParser parser(
        "Usage: download_heightmap --min_lat <min_lat> --min_long <min_long> --max_lat <max_lat> --max_long <max_long>",
        {},
        {"--min_lat", "--min_long", "--max_lat", "--max_long"});
    const auto args = parser.parsed(argc, argv);
    args.assert_num_unamed(0);
    //float min_lat = safe_stof(args.named_value("--min_lat"));
    //float min_long = safe_stof(args.named_value("--min_long"));
    //float max_lat = safe_stof(args.named_value("--max_lat"));
    //float max_long = safe_stof(args.named_value("--max_long"));

    // HTTP
    httplib::Client cli("https://tile.nextzen.org");

    auto res = cli.Get("/tilezen/terrain/v1/512/terrarium/10/10/10.png?api_key=dmlO1fVQRPKI-GrVIYJ1YA");
    res->status;
    res->body;
    std::ofstream ofstr("/tmp/tile.png", std::ios::binary);
    for(char c : res->body) {
        ofstr.put(c);
    }
    ofstr.flush();

    return 0;
}
