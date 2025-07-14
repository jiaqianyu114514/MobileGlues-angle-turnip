//
// Created by BZLZHH on 2025/1/26.
//
#ifndef MOBILEGLUES_SHADER_H
#define MOBILEGLUES_SHADER_H

#include "gl.h"

struct shader_t {
    GLuint id;
    char * converted;
    int frag_data_changed;
};

extern struct shader_t shaderInfo;

GLAPI GLAPIENTRY void glShaderSource(GLuint shader, GLsizei count, const GLchar *const* string, const GLint *length);
GLAPI GLAPIENTRY void glGetShaderiv(GLuint shader, GLenum pname, GLint *params);

#endif //FOLD_CRAFT_LAUNCHER_GL_LOADER_H
