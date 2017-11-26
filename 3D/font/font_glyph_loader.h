/* SPDX-License-Identifier: GPL-3.0 */

#ifndef DE_LAW_EXPLORER_3D_FONT_GLYPH_LOADER_H
#define DE_LAW_EXPLORER_3D_FONT_GLYPH_LOADER_H

#include <cstdint>

#include "font_glyph.h"

class FontGlyphLoader {
  public:
    virtual ~FontGlyphLoader() {};
    virtual FontGlyph* glyph(uint32_t) = 0;
    virtual uint32_t height() const = 0;
    virtual void height(uint32_t) = 0;
};

#endif
