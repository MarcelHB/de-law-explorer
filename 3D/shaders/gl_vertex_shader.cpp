/* SPDX-License-Identifier: GPL-3.0 */

#define USE_GLEW_NOT_QT
#include "gl_vertex_shader.h"

GLVertexShader::GLVertexShader(std::istream &is) :
  GLShader(is) {
}

void GLVertexShader::initialize() {
    this->shader_id = glCreateShader(GL_VERTEX_SHADER);
}
