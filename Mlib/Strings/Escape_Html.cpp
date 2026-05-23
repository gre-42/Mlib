#include "Escape_Html.hpp"

using namespace Mlib;

std::string Mlib::escape_html(std::string_view src) {
    std::string dst;
    dst.reserve(src.size()); // Speicher vorab reservieren
    for (char c : src) {
        switch (c) {
            case '&':  dst.append("&amp;");  break;
            case '<':  dst.append("&lt;");   break;
            case '>':  dst.append("&gt;");   break;
            case '"':  dst.append("&quot;"); break;
            case '\'': dst.append("&apos;"); break;
            default:   dst.push_back(c);     break;
        }
    }
    return dst;
}
