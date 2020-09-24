#pragma once
#include <Mlib/Stats/Linspace.hpp>

namespace Mlib {

class PathDrawer {
public:
    explicit PathDrawer(std::ostream& ostr)
    :ostr_{ostr}
    {
        ostr_ <<
            "  <path "
            "stroke-width=\"1.5\" "
            "stroke=\"#000\" "
            "fill=\"none\" "
            "d=\"M ";
    }
    template <class TData>
    void draw_point(const TData& x, const TData& y) {
        ostr_ << x << "," << y << " ";
    }
    void finish() {
        ostr_ <<
            "\"/>\n";
    }
private:
    std::ostream& ostr_;
};

template <class TData>
struct SvgTransformationParameters {
    TData angle = 0;
    TData rotation_x = 0;
    TData rotation_y = 0;
    TData translation_x = 0;
    TData translation_y = 0;
};

template <class TData>
class SvgTransform {
public:
    explicit SvgTransform(
        std::ostream& ostr,
        const SvgTransformationParameters<TData>& params)
    : ostr_{ostr},
      params_{params}
    {
        ostr << "  <g transform=\""
            "translate(" << params.translation_x
            << " " << params.translation_y
            << ")\n                rotate(" << params.angle
            << " " << params.rotation_x
            << " " << params.rotation_y
            << ")\">\n";
    }
    void finish() {
        ostr_ << "  </g>\n";
    }
private:
    std::ostream& ostr_;
    SvgTransformationParameters<TData> params_;
};

template <class TSize>
class Svg {
public:
    Svg(std::ostream& ostr, const TSize& width, const TSize& height)
    : ostr_{ostr},
      width_{width},
      height_{height}
    {
        ostr <<
            "<svg width=\""
            << width << "\" height=\""
            << height << "\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\">\n";
    }

    void finish() {
        ostr_ << 
            "</svg>\n";
    }

    void draw_background() {
        ostr_ <<
            " <g>\n"
            "  <title>Background</title>\n"
            "  <rect fill=\"#fff\" height=\"" << width_ << "\" width=\"" << height_ << "\" y=\"0\" x=\"0\"/>\n"
            " </g>\n";
    }

    template <class TData>
    void draw_line(const TData& x0, const TData& y0, const TData& x1, const TData& y1) {
        ostr_ <<
            "  <line "
            "y2=\"" << y1 << "\" "
            "x2=\"" << x1 << "\" "
            "y1=\"" << y0 << "\" "
            "x1=\"" << x0 << "\" "
            "stroke-width=\"1.5\" "
            "stroke=\"#000\" "
            "fill=\"none\"/>\n";
    }

    template <class TData>
    void draw_path(const std::vector<TData>& x, const std::vector<TData>& y, size_t down_sampling = 1) {
        if (x.size() != y.size()) {
            throw std::runtime_error("Size mismatch in draw_polygon");
        }
        PathDrawer pd{ostr_};
        for(size_t i = 0; i < x.size(); i += down_sampling) {
            pd.draw_point(x[i], y[i]);
        }
        pd.finish();
    }

    template <class TData>
    void draw_text(const TData& x, const TData& y, const std::string& text, const std::string& color = "black") {
        ostr_ <<
            "  <text x=\"" << x << "\" y=\"" << y << "\" fill=\"" << color << "\">" << text << "</text>\n";
    }

    template <class TSize2>
    void draw_image(const std::string& filename, const TSize2& width, const TSize2& height) {
        ostr_
            << "  <image xlink:href=\"" << filename << "\" "
            << "width=\"" << width << "\" "
            << "height=\"" << height << "\"/>\n";
    }

    template <class TData>
    void plot(const std::vector<TData>& x, const std::vector<TData>& y, size_t down_sampling = 1) {
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
        // for(size_t i = down_sampling; i < x.size(); i += down_sampling) {
        //     draw_line(xpos(x[i-down_sampling]), ypos(y[i-down_sampling]), xpos(x[i]), ypos(y[i]));
        //}
        PathDrawer pd{ostr_};
        for(size_t i = 0; i < x.size(); i += down_sampling) {
            pd.draw_point(xpos(x[i]), ypos(y[i]));
        }
        pd.finish();
        for(const TData& xx : linspace(*xm.first, *xm.second, 5).flat_iterable()) {
            draw_text<TData>(xpos(xx), height_, std::to_string(xx));
        }
        for(const TData& yy : linspace(*ym.first, *ym.second, 5).flat_iterable()) {
            draw_text<TData>(0, ypos(yy), std::to_string(yy));
        }
    }
private:
    std::ostream& ostr_;
    TSize width_;
    TSize height_;
};

}
