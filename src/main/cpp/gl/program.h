//
// Created by hanji on 2025/2/3.
//

#ifndef MOBILEGLUES_PROGRAM_H
#define MOBILEGLUES_PROGRAM_H

#include "gl.h"

GLAPI GLAPIENTRY void glBindFragDataLocation(GLuint program, GLuint color, const GLchar *name);
GLAPI GLAPIENTRY void glLinkProgram(GLuint program);
GLAPI GLAPIENTRY void glGetProgramiv(GLuint program, GLenum pname, GLint *params);

#endif //MOBILEGLUES_PROGRAM_H
