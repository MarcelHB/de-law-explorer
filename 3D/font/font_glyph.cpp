/* SPDX-License-Identifier: GPL-3.0 */

#include "font_glyph.h"

FontGlyph::FontGlyph(uint32_t code, std::vector<unsigned char> &bytes, size_t pxb, size_t rows, size_t bearing_down, size_t bearing_left) :
    glyph_code(code),
    bitmap(bytes),
    row_bytes(bytes.size() / rows),
    pixel_bytes(pxb),
    _bearing_down(bearing_down),
    _bearing_left(bearing_left)
{}

size_t FontGlyph::bearing_down() const {
    return this->_bearing_down;
}

size_t FontGlyph::bearing_left() const {
    return this->_bearing_left;
}

size_t FontGlyph::bytes_per_row() const {
    return this->row_bytes;
}

size_t FontGlyph::bytes_per_pixel() const {
    return this->pixel_bytes;
}

uint32_t FontGlyph::code() const {
    return this->glyph_code;
}

const std::vector<unsigned char>& FontGlyph::data() const {
    return this->bitmap;
}
