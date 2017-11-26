/* SPDX-License-Identifier: GPL-3.0 */

#include <sstream>
#include <string>
#include <vector>

#define USE_GLEW_NOT_QT
#include "gl_shader.h"

GLShader::GLShader(std::istream &_is) :
  shader_id(0),
  is(&_is) {
}

GLShader::~GLShader() {
    this->dispose();
}

GLuint GLShader::id() {
    if(!this->shader_id) {
        this->initialize();
        this->parse();
        this->compile();
    }
    return this->shader_id;
}

void GLShader::dispose() {
    glDeleteShader(this->shader_id);
    this->shader_id = 0;
}

void GLShader::parse() {
    std::stringstream ss;
    std::string s = ss.str();
    char buf[1024] = {0};

    this->is->seekg(0);
    while(this->is->read(&buf[0], sizeof(buf))) {
        s.append(buf, sizeof(buf));
    }
    s.append(buf, this->is->gcount());

    const char *cs = s.c_str();
    glShaderSource(this->shader_id, 1, &cs, NULL);
}

#ifdef DE_LAW_EXPLORER_GL_DEBUG
#include <QDebug>
#endif

void GLShader::compile() {
    glCompileShader(this->shader_id);

#ifdef DE_LAW_EXPLORER_GL_DEBUG
    GLint res = GL_FALSE;
    int log_length = 0;

    glGetShaderiv(this->shader_id, GL_COMPILE_STATUS, &res);
    glGetShaderiv(this->shader_id, GL_INFO_LOG_LENGTH, &log_length);

    if(!res || log_length > 1) {
        std::vector<char> error_vec(log_length+1);
        glGetShaderInfoLog(this->shader_id, log_length, NULL, &error_vec[0]);
        qDebug() << "Shader compilation: " << res;
        qDebug() << "Shader error: " << static_cast<char*>(&error_vec[0]);
    }
#endif
}
