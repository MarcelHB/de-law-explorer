/* SPDX-License-Identifier: GPL-3.0 */

#ifndef DE_LAW_EXPLORER_GL_SHADER_PROGRAM_H
#define DE_LAW_EXPLORER_GL_SHADER_PROGRAM_H

#ifdef USE_GLEW_NOT_QT
#include "../GL/glew.h"
#endif

#include "gl_shader.h"

class GLShaderProgram {
  public:
    GLShaderProgram();
    ~GLShaderProgram();

    void attach_shader(GLShader&);
    void bind_attrib_location(GLuint, const char*);
    void dispose();
    void finalize();
    GLuint uniform_location(const char*);
    GLuint program_id() const;
    void use();

    static void unuse_program();
  private:
    GLuint _program_id;
};

#endif
