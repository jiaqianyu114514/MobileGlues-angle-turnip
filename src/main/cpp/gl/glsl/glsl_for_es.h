#ifndef GLSL_FOR_ES
#define GLSL_FOR_ES
#include <stdio.h>
#include "../includes.h"
#include "../gl.h"
#include "../glcorearb.h"
#include "../log.h"
#include "../gles/loader.h"

#include <string.h>
#ifdef __cplusplus
extern "C" {
#endif
#include "../mg.h"
    
char *GLSLtoGLSLES_1(char *glsl_code, GLenum glsl_type, unsigned int esversion);
char *GLSLtoGLSLES_2(char *glsl_code, GLenum glsl_type, uint essl_version);
int getGLSLVersion(const char* glsl_code);
#ifdef __cplusplus
}
#endif
#endif