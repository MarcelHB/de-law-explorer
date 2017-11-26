/* SPDX-License-Identifier: GPL-3.0 */

#define USE_GLEW_NOT_QT
#include "gl_texture.h"

GLTexture::GLTexture(const size_t _dim, const GLint form, const size_t form_elems,
                     const GLint _type, const size_t type_size) :
  dim(_dim),
  _format(form),
  type(_type),
  tex_id(0),
  _on_gpu(false)
{
    size_t size = _dim * _dim * form_elems * type_size;
    this->_data = std::vector<unsigned char>(size, 0xFF);
}

GLTexture::~GLTexture() {
    this->release_from_gpu();
}

void GLTexture::bind() {
    glBindTexture(GL_TEXTURE_2D, this->tex_id);
}

GLint GLTexture::dimension() const {
    return this->dim;
}

void GLTexture::data(const char *f) {
    this->_data.assign(f, f + this->_data.size());
}

std::vector<unsigned char>& GLTexture::data() {
    return this->_data;
}

GLint GLTexture::format() const {
    return this->_format;
}

bool GLTexture::on_gpu() const {
    return this->_on_gpu;
}

GLint GLTexture::pixel_type() const {
    return this->type;
}

void GLTexture::release_from_gpu() {
    if(this->_on_gpu) {
        glDeleteTextures(1, &(this->tex_id));
        this->_on_gpu = false;
    }
}

GLuint GLTexture::texture_id() const {
    return this->tex_id;
}

void GLTexture::to_gpu() {
    if(!this->_on_gpu) {
        glGenTextures(1, &(this->tex_id));
        this->bind();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->dim, this->dim, 0, this->_format, this->type, &(this->_data[0]));
        this->_on_gpu = true;
    }
}
