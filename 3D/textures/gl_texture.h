/* SPDX-License-Identifier: GPL-3.0 */

#ifndef DE_LAW_EXPLORER_3D_GL_TEXTURE_H
#define DE_LAW_EXPLORER_3D_GL_TEXTURE_H

#include <cstdint>
#include <vector>

#ifdef USE_GLEW_NOT_QT
#include "../GL/glew.h"
#endif

class GLTexture {
  public:
    GLTexture(size_t, GLint, size_t, GLint, size_t);
    virtual ~GLTexture();
    void bind();
    void data(const char*);
    std::vector<unsigned char>& data();
    GLint dimension() const;
    GLint format() const;
    bool on_gpu() const;
    GLint pixel_type() const;
    void release_from_gpu();
    GLuint texture_id() const;
    virtual void to_gpu();
  protected:
    std::vector<unsigned char> _data;
    GLint dim;
    GLint _format;
    GLint type;
    GLuint tex_id;
    bool _on_gpu;
};

#endif
