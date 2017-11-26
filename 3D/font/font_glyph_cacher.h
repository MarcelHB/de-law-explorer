/* SPDX-License-Identifier: GPL-3.0 */

#ifndef DE_LAW_EXPLORER_3D_FONT_GLYPH_CACHER_H
#define DE_LAW_EXPLORER_3D_FONT_GLYPH_CACHER_H

#include <map>
#include <cstdint>

#include "font_glyph_loader.h"

class FontGlyphCacher : public FontGlyphLoader {
  public:
    FontGlyphCacher(FontGlyphLoader&);
    ~FontGlyphCacher();
    void flush();
    FontGlyph* glyph(uint32_t);
    uint32_t height() const;
    void height(uint32_t);
  private:
    FontGlyphLoader* loader;
    std::map<uint32_t, std::map<uint32_t, FontGlyph*>> cache;
    size_t num_elements;

    bool height_positive(uint32_t) const;
    bool code_positive(uint32_t, uint32_t) const;

    FontGlyph* caching_load(uint32_t, uint32_t, bool);
};

#endif
