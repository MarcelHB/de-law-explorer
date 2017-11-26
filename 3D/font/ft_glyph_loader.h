/* SPDX-License-Identifier: GPL-3.0 */

#ifndef DE_LAW_EXPLORER_3D_FT_GLPYH_LOADER_H
#define DE_LAW_EXPLORER_3D_FT_GLPYH_LOADER_H

#include <cstdint>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "font_glyph_loader.h"
#include "font_glyph.h"
#include "../../exception_with_code.h"

class FTGlyphLoader : public FontGlyphLoader {
  public:
    FTGlyphLoader(const char*, uint32_t);
    ~FTGlyphLoader();
    FontGlyph* glyph(uint32_t);
    uint32_t height() const;
    void height(uint32_t);
  private:
    uint32_t nom_height;
    FT_Library ft_library;
    FT_Face face;
};

class FTException : public ExceptionWithCode {
  public:
    FTException(FT_Error);
};

#endif
