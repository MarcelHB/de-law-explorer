/* SPDX-License-Identifier: GPL-3.0 */

#ifndef DE_LAW_EXPLORER_GL_FRAGMENT_SHADER_H
#define DE_LAW_EXPLORER_GL_FRAGMENT_SHADER_H

#include "gl_shader.h"

class GLFragmentShader : public GLShader {
  public:
    GLFragmentShader(std::istream&);
  private:
    void initialize();
};

#endif
