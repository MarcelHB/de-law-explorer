/* SPDX-License-Identifier: GPL-3.0 */

#ifndef BGRA_TEXTURE_H
#define BGRA_TEXTURE_H

#include "gl_texture.h"

class BGRATexture : public GLTexture {
  public:
    BGRATexture(size_t);
    void to_gpu();
};

#endif // BGRA_TEXTURE_H
