//
// Created by Administrator on 2025/1/27.
//

#include "mg.h"

#define DEBUG 0

hardware_t hardware;
gl_state_t gl_state;

FUNC_GL_STATE_SIZEI(proxy_width)
FUNC_GL_STATE_SIZEI(proxy_height)
FUNC_GL_STATE_ENUM(proxy_intformat)

FILE* file;

void start_log() {
    file = fopen(LOG_FILE_PATH, "a");
}

void write_log(const char* format, ...) {
    if (file == NULL) {
        return;
    }
    va_list args;
    va_start(args, format);
    vfprintf(file, format, args);
    va_end(args);
    fprintf(file, "\n");
    // Todo: close file
    //fclose(file);
}

void clear_log() {
    file = fopen(LOG_FILE_PATH, "w");
    if (file == NULL) {
        return;
    }
    fclose(file);
}

GLenum pname_convert(GLenum pname){
    switch (pname) {
        // TODO: Realize GL_TEXTURE_LOD_BIAS for other devices.
        case GL_TEXTURE_LOD_BIAS:
            return GL_TEXTURE_LOD_BIAS_QCOM;
    }
    return pname;
}

GLenum map_tex_target(GLenum target) {
    switch (target) {
        case GL_TEXTURE_1D:
        case GL_TEXTURE_3D:
        case GL_TEXTURE_RECTANGLE_ARB:
            target = GL_TEXTURE_2D;
            break;
        case GL_PROXY_TEXTURE_1D:
        case GL_PROXY_TEXTURE_3D:
        case GL_PROXY_TEXTURE_RECTANGLE_ARB:
            target = GL_PROXY_TEXTURE_2D;
            break;
    }
    return target;
}