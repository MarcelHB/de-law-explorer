/* SPDX-License-Identifier: GPL-3.0 */

#ifndef DE_LAW_EXPLORER_GL_VERTEX_SHADER_H
#define DE_LAW_EXPLORER_GL_VERTEX_SHADER_H

#include "gl_shader.h"

class GLVertexShader : public GLShader {
  public:
    GLVertexShader(std::istream&);
  private:
    void initialize();
};

#endif
