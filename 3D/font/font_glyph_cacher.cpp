/* SPDX-License-Identifier: GPL-3.0 */

#include <iterator>

#include "font_glyph_cacher.h"

FontGlyphCacher::FontGlyphCacher(FontGlyphLoader &_loader) :
  loader(&_loader),
  num_elements(0)
{}

FontGlyphCacher::~FontGlyphCacher() {
    this->flush();
}

void FontGlyphCacher::flush() {
    for(auto it = this->cache.cbegin(); it != this->cache.cend(); ++it) {
        auto height_cache = (*it).second;
        for(auto sub_it = height_cache.cbegin(); sub_it != height_cache.cend(); ++sub_it) {
            FontGlyph* glyph = (*sub_it).second;
            delete glyph;
        }
    }
    this->cache.clear();
    this->num_elements = 0;
}

FontGlyph* FontGlyphCacher::glyph(uint32_t code) {
    uint32_t height = this->height();

    bool has_height = this->height_positive(height);
    if(has_height) {
        bool has_code = this->code_positive(height, code);
        if(has_code) {
            return this->cache.at(height).at(code);
        } else {
            return this->caching_load(height, code, true);
        }
    } else {
        return this->caching_load(height, code, false);
    }
}

bool FontGlyphCacher::height_positive(uint32_t height) const {
    auto it = this->cache.find(height);
    return it != this->cache.end();
}

bool FontGlyphCacher::code_positive(uint32_t height, uint32_t code) const {
    std::map<uint32_t, FontGlyph*> height_cache = this->cache.at(height);
    auto it = height_cache.find(code);
    return it != height_cache.end();
}

/* Always check with height/code_positive first before using this. */
FontGlyph* FontGlyphCacher::caching_load(uint32_t height, uint32_t code,
                                         bool height_avail) {
    if(!height_avail) {
        std::map<uint32_t, FontGlyph*> map;
        this->cache.insert(std::make_pair(height, map));
    }

    FontGlyph *glyph = this->loader->glyph(code);
    this->cache[height].insert(std::make_pair(code, glyph));
    this->num_elements++;

    return glyph;
}

uint32_t FontGlyphCacher::height() const {
    return this->loader->height();
}

void FontGlyphCacher::height(uint32_t _height) {
    this->loader->height(_height);
}
