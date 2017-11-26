/* SPDX-License-Identifier: GPL-3.0 */

#include <cmath>
#include <cstring>
#include <vector>

#include "ft_glyph_loader.h"

FTGlyphLoader::FTGlyphLoader(const char* path, uint32_t _height) {
    FT_Error err = FT_Init_FreeType(&this->ft_library);
    if(err) {
        throw new FTException(err);
    }

    err = FT_New_Face(this->ft_library, path, 0, &this->face);
    if(err) {
        throw new FTException(err);
    }

    this->height(_height);
}

FTGlyphLoader::~FTGlyphLoader() {
    FT_Done_Face(this->face);
}

FontGlyph* FTGlyphLoader::glyph(uint32_t code) {
    /* Non white-space. */
    if(code != 32) {
        uint32_t idx = FT_Get_Char_Index(this->face, code);
        FT_Error err = FT_Load_Glyph(this->face, idx, FT_LOAD_DEFAULT);
        if(err) {
            throw new FTException(err);
        }

        err = FT_Render_Glyph(this->face->glyph, FT_RENDER_MODE_NORMAL);
        if(err) {
            throw new FTException(err);
        }

        FT_Bitmap bmp = this->face->glyph->bitmap;
        size_t wb = abs(bmp.pitch);
        size_t h = bmp.rows;
        /* Offset for chars below/right to main line. */
        size_t bearing_down = h - this->face->glyph->bitmap_top;
        size_t bearing_left = this->face->glyph->bitmap_left;

        std::vector<unsigned char> glyph_tex(h * wb * 4);
        for(size_t j = 0; j < h; ++j) {
            size_t g_off = j * wb;
            for(size_t k = 0; k < wb; ++k) {
                /* Convert 8bit grayscale to RGBA. */
                uint32_t buf = 0;
                memset(&buf, bmp.buffer[g_off + k], 4);
                /* Invert as FT2 seems to load white-on-black. */
                buf ^= 0xFFFFFF;
                buf |= 0xFF000000;
                memcpy(&glyph_tex[(g_off + k) * 4], &buf, 4);
            }
        }

        FontGlyph *glyph = new FontGlyph(code, glyph_tex, 4, h, bearing_down, bearing_left);
        return glyph;
    } else {
        /* Give space a width of 1/3 of height, this should look fine. */
        size_t size = this->nom_height * (this->nom_height / 3) * 4;
        std::vector<unsigned char> glyph_tex(size, 0xFF);
        FontGlyph *glyph = new FontGlyph(code, glyph_tex, 4, this->nom_height, 0, 0);
        return glyph;
    }
}

uint32_t FTGlyphLoader::height() const {
    return this->nom_height;
}

void FTGlyphLoader::height(uint32_t _height) {
    this->nom_height = _height;

    FT_Error err = FT_Set_Pixel_Sizes(this->face, 0, _height);
    if(err) {
        throw new FTException(err);
    }
}

FTException::FTException(FT_Error err) :
  ExceptionWithCode(static_cast<int32_t>(err))
{}
