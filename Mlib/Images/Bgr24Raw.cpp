#include "Bgr24Raw.hpp"
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <fstream>

using namespace Mlib;

Bgr24Raw Bgr24Raw::load_from_file(const std::string& filename) {
    static const DECLARE_REGEX(re, "^.*-(\\d+)x(\\d+)x(\\d+)\\.bgr$");
    Mlib::re::cmatch match;
    if (Mlib::re::regex_match(filename, match, re)) {
        if (match[3] != "24") {
            THROW_OR_ABORT("Only 24-bit raw images are supported");
        }
        std::ifstream istream(filename, std::ios_base::binary);
        try {
            // size is given in width*height, but we deal with rows*columns
            // => swap match 1 and 2
            return load_from_stream(
                ArrayShape{
                    static_cast<size_t>(safe_stoi(match[2].str())),
                    static_cast<size_t>(safe_stoi(match[1].str()))},
                istream);
        } catch (const std::runtime_error& e) {
            THROW_OR_ABORT(e.what() + std::string(": ") + filename);
        }
    } else {
        THROW_OR_ABORT("Filename "+ filename + " does not match ^.*-(\\d+)x(\\d+)x(\\d+)\\.bgr$");
    }
}

Bgr24Raw Bgr24Raw::load_from_stream(const ArrayShape& shape, std::istream& istream) {
    Bgr24Raw result;
    result.do_resize(shape);
    istream.read(reinterpret_cast<char*>(&result(0, 0)), integral_cast<std::streamsize>(result.nbytes()));
    if (istream.fail()) {
        THROW_OR_ABORT("Could not read raw image");
    }
    return result;
}

Array<float> Bgr24Raw::to_float_grayscale() const {
    Array<float> grayscale(shape());
    Array<Bgr24> f = flattened();
    Array<float> g = grayscale.flattened();
    for (size_t i = 0; i < g.length(); i++) {
        g(i) = (
            static_cast<float>(f(i).r) / 255 +
            static_cast<float>(f(i).g) / 255 +
            static_cast<float>(f(i).b) / 255) / 3;
        assert(g(i) >= 0);
        assert(g(i) <= 1);
    }
    return grayscale;
}

Array<float> Bgr24Raw::to_float_rgb() const {
    Array<float> result(ArrayShape{3}.concatenated(shape()));
    Array<float> R = result[0];
    Array<float> G = result[1];
    Array<float> B = result[2];
    for (size_t r = 0; r < shape(0); ++r) {
        for (size_t c = 0; c < shape(1); ++c) {
            R(r, c) = static_cast<float>((*this)(r, c).r) / 255;
            G(r, c) = static_cast<float>((*this)(r, c).g) / 255;
            B(r, c) = static_cast<float>((*this)(r, c).b) / 255;
        }
    }
    return result;
}
