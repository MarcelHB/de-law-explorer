/* SPDX-License-Identifier: GPL-3.0 */

#define USE_GLEW_NOT_QT
#include "gl_fragment_shader.h"

GLFragmentShader::GLFragmentShader(std::istream &is) :
  GLShader(is) {
}

void GLFragmentShader::initialize() {
    this->shader_id = glCreateShader(GL_FRAGMENT_SHADER);
}
