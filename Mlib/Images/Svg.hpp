#pragma once
#include <Mlib/Math/Funpack.hpp>
#include <Mlib/Stats/Linspace.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <algorithm>
#include <iomanip>
#include <list>
#include <sstream>

namespace Mlib {

template <class TData>
inline std::string to_string_precision(const TData& d, int n) {
    std::stringstream sstr;
    sstr << std::setprecision(n) << d;
    return sstr.str();
}

template <class TData>
class PathDrawer {
public:
    explicit PathDrawer(
        std::ostream& ostr,
        const TData& stroke_width,
        const std::string& stroke = "#000",
        const std::string& fill = "none")
    :ostr_{ostr}
    {
        ostr_ <<
            "  <path "
            "stroke-width=\"" << stroke_width << "\" "
            "stroke=\"" << stroke << "\" "
            "fill=\"" << fill << "\" "
            "d=\"M ";
    }
    void draw_point(const TData& x, const TData& y) {
        ostr_ << x << "," << y << " ";
    }
    void finish(bool close_path = false) {
        if (close_path) {
            ostr_ << " z";
        }
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
        const std::string& color = "black",
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
            "stroke=\"" << color << "\" "
            "fill=\"none\"/>\n";
    }

    template <class TData>
    void draw_path(
        const std::vector<TData>& x,
        const std::vector<TData>& y,
        const TData& stroke_width = 1.5,
        const std::string& stroke = "#000",
        const std::string& fill = "none",
        bool close = false,
        size_t down_sampling = 1)
    {
        if (x.size() != y.size()) {
            THROW_OR_ABORT("Size mismatch in draw_path");
        }
        PathDrawer pd{ostr_, stroke_width, stroke, fill};
        for (size_t i = 0; i < x.size(); i += down_sampling) {
            pd.draw_point(x[i], y[i]);
        }
        pd.finish(close);
    }

    template <class TData>
    void draw_rectangle(
        const TData& x0,
        const TData& y0,
        const TData& x1,
        const TData& y1,
        const TData& stroke_width = (TData)1.5f,
        const std::string& fill = "black",
        const TData& fill_opacity = (TData)0.1f)
    {
        ostr_ <<
            "  <rect x=\"" << x0 <<
            "\" y=\"" << y0 <<
            "\" width=\"" << (x1 - x0) <<
            "\" height=\"" << (y1 - y0) <<
            "\" style=\"fill:" << fill <<
            ";fill-opacity:" << fill_opacity <<
            ";stroke:black;stroke-width:" << stroke_width <<
            "\" />\n";
    }

    template <class TData>
    void draw_text(const TData& x, const TData& y, const std::string& text, const std::string& color = "black", const std::string& anchor = "start") {
        ostr_ <<
            "  <text text-anchor=\"" << anchor <<
            "\" x=\"" << x <<
            "\" y=\"" << y <<
            "\" fill=\"" << color << "\">" <<
            text << "</text>\n";
    }

    template <class TSize2>
    void draw_image(const std::string& filename, const TSize2& width, const TSize2& height) {
        ostr_ <<
            "  <image xlink:href=\"" << filename << "\" " <<
            "width=\"" << width << "\" " <<
            "height=\"" << height << "\"/>\n";
    }

    template <class TData>
    void plot_multiple(
        const std::vector<std::vector<TData>>& x,
        const std::vector<std::vector<TData>>& y,
        const funpack_t<TData>& stroke_width = 1.5,
        const std::vector<std::string>& colors = {"#000", "#FF5733"},
        size_t down_sampling = 1)
    {
        if (x.size() != y.size()) {
            THROW_OR_ABORT("Size mismatch in plot");
        }
        if (x.empty()) {
            return;
        }
        auto xm = std::make_pair((const TData*)nullptr, (const TData*)nullptr);
        auto ym = std::make_pair((const TData*)nullptr, (const TData*)nullptr);
        for (size_t i = 0; i < x.size(); ++i) {
            if (x[i].size() != y[i].size()) {
                THROW_OR_ABORT("Size mismatch in plot");
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
            return height_ - ((y - *ym.first) * height_) / std::max((TData)1e-12, *ym.second - *ym.first);
        };
        // for (size_t i = down_sampling; i < x.size(); i += down_sampling) {
        //     draw_line(xpos(x[i-down_sampling]), ypos(y[i-down_sampling]), xpos(x[i]), ypos(y[i]));
        //}
        for (size_t i = 0; i < x.size(); ++i) {
            if (colors.empty()) {
                THROW_OR_ABORT("No color defined");
            }
            PathDrawer pd{ostr_, stroke_width, colors[i % colors.size()]};
            for (size_t j = 0; j < x[i].size(); j += down_sampling) {
                pd.draw_point(xpos(x[i][j]), ypos(y[i][j]));
            }
            pd.finish();
        }
        for (const TData& xx : Linspace{ *xm.first, *xm.second, 5 }) {
            draw_text<TData>(xpos(xx), height_, std::to_string(xx));
        }
        for (const TData& yy : Linspace{ *ym.first, *ym.second, 5 }) {
            draw_text<TData>(0, ypos(yy), std::to_string(yy));
        }
    }

    template <class TData>
    void plot(
        const std::vector<TData>& x,
        const std::vector<TData>& y,
        const funpack_t<TData>& stroke_width = 1.5f,
        size_t down_sampling = 1)
    {
        using I = funpack_t<TData>;
        if (x.size() != y.size()) {
            THROW_OR_ABORT("Size mismatch in plot");
        }
        if (x.empty()) {
            return;
        }
        const auto xm = std::minmax_element(x.begin(), x.end());
        const auto ym = std::minmax_element(y.begin(), y.end());
        const auto xpos = [&](const I& x) {
            return ((x - funpack(*xm.first)) * width_) / (funpack(*xm.second - *xm.first));
        };
        const auto ypos = [&](const I& y) {
            return height_ - ((y - funpack(*ym.first)) * height_) / funpack(*ym.second - *ym.first);
        };
        // for (size_t i = down_sampling; i < x.size(); i += down_sampling) {
        //     draw_line(xpos(x[i-down_sampling]), ypos(y[i-down_sampling]), xpos(x[i]), ypos(y[i]));
        //}
        PathDrawer pd{ ostr_, stroke_width };
        for (size_t i = 0; i < x.size(); i += down_sampling) {
            pd.draw_point(xpos(funpack(x[i])), ypos(funpack(y[i])));
        }
        pd.finish();
        for (const I& xx : Linspace{ funpack(*xm.first), funpack(*xm.second), 5 }) {
            draw_text<I>(xpos(xx), height_, std::to_string(xx));
        }
        for (const I& yy : Linspace{ funpack(*ym.first), funpack(*ym.second), 5 }) {
            draw_text<I>(0, ypos(yy), std::to_string(yy));
        }
    }

    template <class TData>
    void plot(
        const std::list<TData>& x,
        const std::list<TData>& y,
        const funpack_t<TData>& stroke_width = 1.5,
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
            THROW_OR_ABORT("Size mismatch in plot");
        }
        if (x_start.size() != x_stop.size()) {
            THROW_OR_ABORT("Size mismatch in plot");
        }
        if (x_start.size() != y_stop.size()) {
            THROW_OR_ABORT("Size mismatch in plot");
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
                "black",
                "line" + std::to_string(i));
        }
        for (const TData& xx : Linspace{ xm_min, xm_max, 5 }) {
            draw_text<TData>(xpos(xx), height_, std::to_string(xx));
        }
        for (const TData& yy : Linspace{ ym_min, ym_max, 5 }) {
            draw_text<TData>(0, ypos(yy), std::to_string(yy));
        }
    }

    template <class TData>
    void plot_waveforms(
        const std::vector<TData>& x,
        const std::vector<std::vector<bool>>& ys,
        const std::vector<std::string>& labels,
        const TData& stroke_width = 1.5,
        const std::vector<std::string>& colors = {"#000", "#FF5733"},
        size_t down_sampling = 1,
        TData xoffset = (TData)20,
        TData offset = (TData)0.5,
        TData amplitude = (TData)1,
        TData spacing = (TData)1,
        int precision = 2)
    {
        if (ys.size() != labels.size()) {
            THROW_OR_ABORT("Number of waveforms differs from number of labels");
        }
        if (x.empty()) {
            return;
        }
        const auto xm = std::minmax_element(x.begin(), x.end());
        TData ymin = 0.f;
        TData ymax = (amplitude + spacing) * (TData)ys.size();
        const auto xpos = [&](const TData& x) {
            return ((x - *xm.first) * (width_ - xoffset)) / (*xm.second - *xm.first) + xoffset;
        };
        const auto ypos = [&](const TData& y) {
            return height_ - ((y - ymin) * height_) / (ymax - ymin);
        };
        // for (size_t i = down_sampling; i < x.size(); i += down_sampling) {
        //     draw_line(xpos(x[i-down_sampling]), ypos(y[i-down_sampling]), xpos(x[i]), ypos(y[i]));
        //}
        if (colors.empty()) {
            THROW_OR_ABORT("No color defined");
        }
        for (size_t i = 0; i < ys.size(); ++i) {
            TData height = ymin + TData(i) * (ymax - ymin) / (TData)ys.size() + offset;
            if (x.size() != ys[i].size()) {
                THROW_OR_ABORT("Size mismatch in plot waveforms");
            }
            PathDrawer pd{ostr_, stroke_width, colors[i % colors.size()]};
            for (size_t j = 0; j < x.size(); j += down_sampling) {
                if ((j > 0) && (ys[i][j] != ys[i][j - 1])) {
                    pd.draw_point(xpos(x[j]), ypos(amplitude * ys[i][j - 1] + height));
                }
                pd.draw_point(xpos(x[j]), ypos(amplitude * ys[i][j] + height));
            }
            pd.finish();
            draw_text<TData>(0, ypos(height + TData(0.5) * amplitude), labels[i]);
        }
        for (const TData& xx : Linspace{ *xm.first, *xm.second, 5 }) {
            draw_text<TData>(xpos(xx), height_, to_string_precision(xx, precision), "black", "end");
        }
    }

    const TSize& width() const {
        return width_;
    }

    const TSize& height() const {
        return height_;
    }
private:
    std::ostream& ostr_;
    TSize width_;
    TSize height_;
};

}
