//
// Created by Swung0x48 on 2024/10/8.
//

#include "../includes.h"
#include "gl.h"
#include "glcorearb.h"
#include "log.h"
#include "../gles/loader.h"
#include "mg.h"

#define DEBUG 0

GLAPI GLAPIENTRY void glClearDepth(GLclampd depth) {
    LOG();
    glClearDepthf(depth);
}

void glHint(GLenum target, GLenum mode) {
    LOG();
}