#pragma once
#include <Mlib/Stats/Linspace.hpp>
#include <list>

namespace Mlib {

template <class TData>
class PathDrawer {
public:
    explicit PathDrawer(
        std::ostream& ostr,
        const TData& stroke_width,
        const std::string& stroke = "#000")
    :ostr_{ostr}
    {
        ostr_ <<
            "  <path "
            "stroke-width=\"" << stroke_width << "\" "
            "stroke=\"" << stroke << "\" "
            "fill=\"none\" "
            "d=\"M ";
    }
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
    void draw_line(
        const TData& x0,
        const TData& y0,
        const TData& x1,
        const TData& y1,
        const TData& stroke_width = 1.5,
        const std::string& id = "")
    {
        ostr_ <<
            "  <line " <<
            (id.empty() ? "" : "id=\"" + id + "\" ") <<
            "y2=\"" << y1 << "\" "
            "x2=\"" << x1 << "\" "
            "y1=\"" << y0 << "\" "
            "x1=\"" << x0 << "\" "
            "stroke-width=\"" << stroke_width << "\" "
            "stroke=\"#000\" "
            "fill=\"none\"/>\n";
    }

    template <class TData>
    void draw_path(
        const std::vector<TData>& x,
        const std::vector<TData>& y,
        const TData& stroke_width = 1.5,
        size_t down_sampling = 1)
    {
        if (x.size() != y.size()) {
            throw std::runtime_error("Size mismatch in draw_path");
        }
        PathDrawer pd{ostr_, stroke_width};
        for (size_t i = 0; i < x.size(); i += down_sampling) {
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
    void plot(
        const std::vector<std::vector<TData>>& x,
        const std::vector<std::vector<TData>>& y,
        const TData& stroke_width = 1.5,
        const std::vector<std::string>& colors = {"#000", "#FF5733"},
        size_t down_sampling = 1)
    {
        if (x.size() != y.size()) {
            throw std::runtime_error("Size mismatch in plot");
        }
        if (x.empty()) {
            return;
        }
        auto xm = std::make_pair((const TData*)nullptr, (const TData*)nullptr);
        auto ym = std::make_pair((const TData*)nullptr, (const TData*)nullptr);
        for (size_t i = 0; i < x.size(); ++i) {
            if (x[i].size() != y[i].size()) {
                throw std::runtime_error("Size mismatch in plot");
            }
            if (x[i].empty()) {
                continue;
            }
            const auto lxm = std::minmax_element(x[i].begin(), x[i].end());
            const auto lym = std::minmax_element(y[i].begin(), y[i].end());
            if (xm.first == nullptr) {
                xm.first = &*lxm.first;
                xm.second = &*lxm.second;
                ym.first = &*lym.first;
                ym.second = &*lym.second;
            } else {
                xm.first = &std::min(*xm.first, *lxm.first);
                xm.second = &std::max(*xm.second, *lxm.second);
                ym.first = &std::min(*ym.first, *lym.first);
                ym.second = &std::max(*ym.second, *lym.second);
            }
        }
        if (xm.first == nullptr) {
            return;
        }
        const auto xpos = [&](const TData& x) {
            return ((x - *xm.first) * width_) / (*xm.second - *xm.first);
        };
        const auto ypos = [&](const TData& y) {
            return height_ - ((y - *ym.first) * height_) / (*ym.second - *ym.first);
        };
        // for (size_t i = down_sampling; i < x.size(); i += down_sampling) {
        //     draw_line(xpos(x[i-down_sampling]), ypos(y[i-down_sampling]), xpos(x[i]), ypos(y[i]));
        //}
        for (size_t i = 0; i < x.size(); ++i) {
            if (colors.empty()) {
                throw std::runtime_error("No color defined");
            }
            PathDrawer pd{ostr_, stroke_width, colors[i % colors.size()]};
            for (size_t j = 0; j < x[i].size(); j += down_sampling) {
                pd.draw_point(xpos(x[i][j]), ypos(y[i][j]));
            }
            pd.finish();
        }
        for (const TData& xx : linspace(*xm.first, *xm.second, 5).flat_iterable()) {
            draw_text<TData>(xpos(xx), height_, std::to_string(xx));
        }
        for (const TData& yy : linspace(*ym.first, *ym.second, 5).flat_iterable()) {
            draw_text<TData>(0, ypos(yy), std::to_string(yy));
        }
    }

    template <class TData>
    void plot(
        const std::vector<TData>& x,
        const std::vector<TData>& y,
        const TData& stroke_width = 1.5,
        size_t down_sampling = 1)
    {
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
        // for (size_t i = down_sampling; i < x.size(); i += down_sampling) {
        //     draw_line(xpos(x[i-down_sampling]), ypos(y[i-down_sampling]), xpos(x[i]), ypos(y[i]));
        //}
        PathDrawer pd{ostr_, stroke_width};
        for (size_t i = 0; i < x.size(); i += down_sampling) {
            pd.draw_point(xpos(x[i]), ypos(y[i]));
        }
        pd.finish();
        for (const TData& xx : linspace(*xm.first, *xm.second, 5).flat_iterable()) {
            draw_text<TData>(xpos(xx), height_, std::to_string(xx));
        }
        for (const TData& yy : linspace(*ym.first, *ym.second, 5).flat_iterable()) {
            draw_text<TData>(0, ypos(yy), std::to_string(yy));
        }
    }

    template <class TData>
    void plot(
        const std::list<TData>& x,
        const std::list<TData>& y,
        const TData& stroke_width = 1.5,
        size_t down_sampling = 1)
    {
        plot(std::vector(x.begin(), x.end()), std::vector(y.begin(), y.end()), stroke_width, down_sampling);
    }

    template <class TData>
    void plot_edges(
        const std::vector<TData>& x_start,
        const std::vector<TData>& y_start,
        const std::vector<TData>& x_stop,
        const std::vector<TData>& y_stop,
        const TData& stroke_width = 1.5,
        size_t down_sampling = 1)
    {
        if (x_start.size() != y_start.size()) {
            throw std::runtime_error("Size mismatch in plot");
        }
        if (x_start.size() != x_stop.size()) {
            throw std::runtime_error("Size mismatch in plot");
        }
        if (x_start.size() != y_stop.size()) {
            throw std::runtime_error("Size mismatch in plot");
        }
        if (x_start.empty()) {
            return;
        }
        const auto xm_start = std::minmax_element(x_start.begin(), x_start.end());
        const auto xm_stop = std::minmax_element(x_stop.begin(), x_stop.end());
        const auto ym_start = std::minmax_element(y_start.begin(), y_start.end());
        const auto ym_stop = std::minmax_element(y_stop.begin(), y_stop.end());
        const auto xm_min = std::min(*xm_start.first, *xm_stop.first);
        const auto xm_max = std::max(*xm_start.second, *xm_stop.second);
        const auto ym_min = std::min(*ym_start.first, *ym_stop.first);
        const auto ym_max = std::max(*ym_start.second, *ym_stop.second);
        const auto xpos = [&](const TData& x) {
            return ((x - xm_min) * width_) / (xm_max - xm_min);
        };
        const auto ypos = [&](const TData& y) {
            return height_ - ((y - ym_min) * height_) / (ym_max - ym_min);
        };
        // for (size_t i = down_sampling; i < x.size(); i += down_sampling) {
        //     draw_line(xpos(x[i-down_sampling]), ypos(y[i-down_sampling]), xpos(x[i]), ypos(y[i]));
        //}
        for (size_t i = 0; i < x_start.size(); i += down_sampling) {
            // PathDrawer pd{ostr_};
            // pd.draw_point(xpos(x_start[i]), ypos(y_start[i]));
            // pd.draw_point(xpos(x_stop[i]), ypos(y_stop[i]));
            // pd.finish();
            draw_line(
                xpos(x_start[i]), ypos(y_start[i]),
                xpos(x_stop[i]), ypos(y_stop[i]),
                stroke_width,
                "line" + std::to_string(i));
        }
        for (const TData& xx : linspace(xm_min, xm_max, 5).flat_iterable()) {
            draw_text<TData>(xpos(xx), height_, std::to_string(xx));
        }
        for (const TData& yy : linspace(ym_min, ym_max, 5).flat_iterable()) {
            draw_text<TData>(0, ypos(yy), std::to_string(yy));
        }
    }
private:
    std::ostream& ostr_;
    TSize width_;
    TSize height_;
};

}
