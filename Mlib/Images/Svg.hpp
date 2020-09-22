#pragma once
#include <Mlib/Stats/Linspace.hpp>

namespace Mlib {

class Svg {
public:
    Svg(std::ostream& ostr, size_t width, size_t height)
    : ostr_{ostr},
      width_{width},
      height_{height}
    {
        ostr <<
            "<svg width=\"" << width << "\" height=\"" << height << "\" xmlns=\"http://www.w3.org/2000/svg\">\n"
            " <g>\n"
            "  <title>Background</title>\n"
            "  <rect fill=\"#fff\" height=\"" << width << "\" width=\"" << height << "\" y=\"0\" x=\"0\"/>\n"
            " </g>\n";
    }

    void finish() {
        ostr_ << 
            "</svg>\n";
    }

    template <class TData>
    void draw_line(const TData& x0, const TData& y0, const TData& x1, const TData& y1) {
        ostr_ <<
            "  <line stroke-linecap=\"undefined\" stroke-linejoin=\"undefined\" "
            "y2=\"" << y1 << "\" "
            "x2=\"" << x1 << "\" "
            "y1=\"" << y0 << "\" "
            "x1=\"" << x0 << "\" "
            "stroke-width=\"1.5\" "
            "stroke=\"#000\" "
            "fill=\"none\"/>\n";
    }

    template <class TData>
    void draw_text(const TData& x, const TData& y, const std::string& text, const std::string& color = "black") {
        ostr_ <<
            "  <text x=\"" << x << "\" y=\"" << y << "\" fill=\"" << color << "\">" << text << "</text>\n";
    }

    template <class TData>
    void plot(const std::vector<TData>& x, const std::vector<TData>& y) {
        if (x.size() != y.size()) {
            throw std::runtime_error("Size mismatch in plot");
        }
        if (x.empty()) {
            return;
        }
        const auto xm = std::minmax_element(x.begin(), x.end());
        const auto ym = std::minmax_element(y.begin(), y.end());
        const auto xpos = [&](const TData& x) {
            return ((x - *xm.first) * width_) / (*xm.second - *xm.first);
        };
        const auto ypos = [&](const TData& y) {
            return height_ - ((y - *ym.first) * height_) / (*ym.second - *ym.first);
        };
        for(size_t i = 1; i < x.size(); ++i) {
            draw_line(xpos(x[i-1]), ypos(y[i-1]), xpos(x[i]), ypos(y[i]));
        }
        for(const TData& xx : linspace(*xm.first, *xm.second, 5).flat_iterable()) {
            draw_text<TData>(xpos(xx), height_, std::to_string(xx));
        }
        for(const TData& yy : linspace(*ym.first, *ym.second, 5).flat_iterable()) {
            draw_text<TData>(0, ypos(yy), std::to_string(yy));
        }
    }
private:
    std::ostream& ostr_;
    size_t width_;
    size_t height_;
};

}
