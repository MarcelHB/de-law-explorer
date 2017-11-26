/* SPDX-License-Identifier: GPL-3.0 */

#define USE_GLEW_NOT_QT
#include "bgra_texture.h"

BGRATexture::BGRATexture(size_t dim) :
  GLTexture(dim, GL_BGRA, 4, GL_UNSIGNED_BYTE, 1)
{ }

void BGRATexture::to_gpu() {
    if(!this->_on_gpu) {
        glGenTextures(1, &(this->tex_id));
        this->bind();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->dim, this->dim, 0, this->_format, this->type, &(this->_data[0]));

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);

        this->_on_gpu = true;
    }
}
