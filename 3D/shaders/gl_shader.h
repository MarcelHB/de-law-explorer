/* SPDX-License-Identifier: GPL-3.0 */

#ifndef DE_LAW_EXPLORER_GL_SHADER_H
#define DE_LAW_EXPLORER_GL_SHADER_H

#include <istream>

#ifdef USE_GLEW_NOT_QT
#include "../GL/glew.h"
#endif

class GLShader {
  public:
    GLShader(std::istream&);
    virtual ~GLShader();

    GLuint id();
    void dispose();
  protected:
    GLuint shader_id;
    std::istream *is;

    void compile();

    virtual void initialize() = 0;
    virtual void parse();
};

#endif
