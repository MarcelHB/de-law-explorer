/* SPDX-License-Identifier: GPL-3.0 */

#ifndef DE_LAW_EXPLORER_3D_FONT_GLYPH_H
#define DE_LAW_EXPLORER_3D_FONT_GLYPH_H

#include <vector>
#include <cstdint>
#include <cstddef>

class FontGlyph {
  public:
    FontGlyph(uint32_t, std::vector<unsigned char>&, size_t, size_t, size_t, size_t);

    size_t bearing_down() const;
    size_t bearing_left() const;
    size_t bytes_per_row() const;
    size_t bytes_per_pixel() const;
    uint32_t code() const;
    const std::vector<unsigned char>& data() const;
  private:
    uint32_t glyph_code;
    std::vector<unsigned char> bitmap;
    size_t row_bytes;
    size_t pixel_bytes;
    size_t _bearing_down;
    size_t _bearing_left;
};

#endif
