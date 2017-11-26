/* SPDX-License-Identifier: GPL-3.0 */

#define USE_GLEW_NOT_QT
#include "gl_shader_program.h"

GLShaderProgram::GLShaderProgram() {
    this->_program_id = glCreateProgram();
}

GLShaderProgram::~GLShaderProgram() {
    this->dispose();
}

void GLShaderProgram::attach_shader(GLShader &s) {
    glAttachShader(this->_program_id, s.id());
}

void GLShaderProgram::bind_attrib_location(GLuint idx, const char *name) {
    glBindAttribLocation(this->_program_id, idx, name);
}

void GLShaderProgram::dispose() {
    glDeleteProgram(this->_program_id);
    this->_program_id = 0;
}

#ifdef DE_LAW_EXPLORER_GL_DEBUG
#include <QDebug>
#endif

void GLShaderProgram::finalize() {
    glLinkProgram(this->_program_id);

#ifdef DE_LAW_EXPLORER_GL_DEBUG
    GLint res = GL_FALSE;
    int log_length = 0;

    glGetProgramiv(this->_program_id, GL_LINK_STATUS, &res);
    glGetProgramiv(this->_program_id, GL_INFO_LOG_LENGTH, &log_length);

    if(!res || log_length > 1) {
        std::vector<char> error_vec(log_length+1);
        glGetProgramInfoLog(this->_program_id, log_length, NULL, &error_vec[0]);
        qDebug() << "Shader program linking: " << res;
        qDebug() << "Linking error: " << static_cast<char*>(&error_vec[0]);
    }
#endif
}

GLuint GLShaderProgram::uniform_location(const char *name) {
    return glGetUniformLocation(this->_program_id, name);
}

GLuint GLShaderProgram::program_id() const {
    return this->_program_id;
}

void GLShaderProgram::use() {
    glUseProgram(this->_program_id);
}

void GLShaderProgram::unuse_program() {
    glUseProgram(0);
}
