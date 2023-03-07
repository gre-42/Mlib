#include <Mlib/Throw_Or_Abort.hpp>
#include <algorithm>
#include <cassert>
#include <stb/stb_truetype.h>

// Copy&paste from stbtt_BakeFontBitmap_internal,
// but returns the distance of the lowest character
// to the baseline.
float stbtt_BakeFontBitmap_get_y0(
    unsigned char *data, int offset,        // font location (use offset=0 for plain .ttf)
    float pixel_height,                     // height of font in pixels
    unsigned char *pixels, int pw, int ph,  // bitmap to be filled in
    int first_char, int num_chars,          // characters to bake
    stbtt_bakedchar *chardata)
{
    float scale;
    int x, y, bottom_y, i;
    stbtt_fontinfo f;
    f.userdata = nullptr;
    if (!stbtt_InitFont(&f, data, offset)) {
        THROW_OR_ABORT("Could not initialize font");
    }
    // background of 0 around pixels
    std::fill(pixels, pixels + pw * ph, 0);
    x=y=1;
    bottom_y = 1;

    scale = stbtt_ScaleForPixelHeight(&f, pixel_height);

    for (i=0; i < num_chars; ++i) {
        int advance, lsb, x0, y0, x1, y1, gw, gh;
        int g = stbtt_FindGlyphIndex(&f, first_char + i);
        stbtt_GetGlyphHMetrics(&f, g, &advance, &lsb);
        stbtt_GetGlyphBitmapBox(&f, g, scale, scale, &x0, &y0, &x1, &y1);
        gw = x1-x0;
        gh = y1-y0;
        if (x + gw + 1 >= pw)
            y = bottom_y, x = 1; // advance to next row
        if (y + gh + 1 >= ph) {// check if it fits vertically AFTER potentially moving to next row
            THROW_OR_ABORT("Bitmap too small for font and size");
        }
        assert(x + gw < pw);
        assert(y + gh < ph);
        stbtt_MakeGlyphBitmap(&f, pixels + x + y * pw, gw, gh, pw, scale,scale, g);
        chardata[i].x0 = (unsigned short) x;
        chardata[i].y0 = (unsigned short) y;
        chardata[i].x1 = (unsigned short) (x + gw);
        chardata[i].y1 = (unsigned short) (y + gh);
        chardata[i].xadvance = scale * (float)advance;
        chardata[i].xoff     = (float) x0;
        chardata[i].yoff     = (float) y0;
        x = x + gw + 1;
        if (y+gh+1 > bottom_y) {
            bottom_y = y+gh+1;
        }
    }
    int x0, y0, x1, y1;
    stbtt_GetFontBoundingBox(&f, &x0, &y0, &x1, &y1);
    return scale * (float)y0;
}
