//
// Created by BZLZHH on 2025/1/27.
//

#include "texture.h"

#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <unordered_map>
#include <android/log.h>

#include "gl.h"
#include "../gles/gles.h"
#include "log.h"
#include "../gles/loader.h"
#include "../includes.h"
#include "glsl/glsl_for_es.h"
#include "mg.h"

#define DEBUG 0

int nlevel(int size, int level) {
    if(size) {
        size>>=level;
        if(!size) size=1;
    }
    return size;
}

static bool support_rgba16 = false;
static bool checked_rgba16 = false;

std::unordered_map<GLuint, texture_t> g_textures;
GLuint bound_texture = 0;

bool check_rgba16() {
    LOAD_GLES(glGetStringi, const GLubyte *, GLenum, GLuint);
    LOAD_GLES(glGetIntegerv, void, GLenum pname, GLint *params);

    GLint numFormats = 0;
    gles_glGetIntegerv(GL_NUM_EXTENSIONS, &numFormats);

    for (int i = 0; i < numFormats; ++i) {
        const GLubyte* extension = gles_glGetStringi(GL_EXTENSIONS, i);
        if (strcmp((const char*)extension, "GL_EXT_texture_norm16") == 0) {
            return true;
        }
    }

    return false;
}

void internal_convert(GLenum* internal_format, GLenum* type, GLenum* format) {
    if (format && *format == GL_BGRA)
        *format = GL_RGBA;
    switch (*internal_format) {
        case GL_DEPTH_COMPONENT16:
            if(type)
                *type = GL_UNSIGNED_SHORT;
            break;

        case GL_DEPTH_COMPONENT24:
            if(type)
                *type = GL_UNSIGNED_INT;
            break;

        case GL_DEPTH_COMPONENT32:
            *internal_format = GL_DEPTH_COMPONENT32F;
            if(type)
                *type = GL_FLOAT;
            break;

        case GL_DEPTH_COMPONENT32F:
            if(type)
                *type = GL_FLOAT;
            break;

        case GL_DEPTH_COMPONENT:
            *internal_format = GL_DEPTH_COMPONENT32F;
            if(type)
                *type = GL_FLOAT;
            break;

        case GL_DEPTH_STENCIL:
            *internal_format = GL_DEPTH32F_STENCIL8;
            if(type)
                *type = GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
            break;

        case GL_RGB10_A2:
            if(type)
                *type = GL_UNSIGNED_INT_2_10_10_10_REV;
            break;

        case GL_RGB5_A1:
            if(type)
                *type = GL_UNSIGNED_SHORT_5_5_5_1;
            break;

        case GL_COMPRESSED_RED_RGTC1:
        case GL_COMPRESSED_RG_RGTC2:
            LOG_E("GL_COMPRESSED_RED_RGTC1 or GL_COMPRESSED_RG_RGTC2 is not supported!")
            break;

        case GL_SRGB8:
            if(type)
                *type = GL_UNSIGNED_BYTE;
            break;

        case GL_RGBA32F:
        case GL_RGB32F:
        case GL_RG32F:
        case GL_R32F:
            if(type)
                *type = GL_FLOAT;
            break;

        case GL_RGB9_E5:
            if(type)
                *type = GL_UNSIGNED_INT_5_9_9_9_REV;
            break;
            
        case GL_R11F_G11F_B10F:
            if(type)
                *type = GL_UNSIGNED_INT_10F_11F_11F_REV;
            if (format)
                *format = GL_RGB;
            break;

        case GL_RGBA32UI:
        case GL_RGB32UI:
        case GL_RG32UI:
        case GL_R32UI:
            if(type)
                *type = GL_UNSIGNED_INT;
            break;

        case GL_RGBA32I:
        case GL_RGB32I:
        case GL_RG32I:
        case GL_R32I:
            if(type)
                *type = GL_INT;
            break;

        case GL_RGBA16: {
            if (!checked_rgba16) {
                support_rgba16 = check_rgba16();
                checked_rgba16 = true;
            }
            if (support_rgba16) {
                if(type)
                    *type = GL_UNSIGNED_SHORT;
            } else {
                *internal_format = GL_RGBA16F;
                if(type)
                    *type = GL_FLOAT;
            }
            break;
        }
        case GL_RGBA8:
            if(type)
                *type = GL_UNSIGNED_BYTE;
            if (format)
                *format = GL_RGBA;
            break;

        case GL_RGBA:
            if(type)
                *type = GL_UNSIGNED_BYTE;
            if (format)
                *format = GL_RGBA;
            break;
            
        case GL_RGBA16F:
        case GL_R16F:
            if(type)
                *type = GL_HALF_FLOAT;
            break;

        case GL_R16:
            *internal_format = GL_R16F;
            if(type)
                *type = GL_FLOAT;
            break;

        case GL_RGB16:
            *internal_format = GL_RGB16F;
            if(type)
                *type = GL_HALF_FLOAT;
            if(format)
                *format = GL_RGB;
            break;
            
        case GL_RGB16F:
            if(type)
                *type = GL_HALF_FLOAT;
            if(format)
                *format = GL_RGB;
            break;

        case GL_RG16F:
            if(type)
                *type = GL_HALF_FLOAT;
            if(format)
                *format = GL_RG;
            break;

        case GL_R8:
            if (format)
                *format = GL_RED;
            if(type)
                *type = GL_UNSIGNED_BYTE;
            break;
        case GL_R8UI:
            if (format)
                *format = GL_RED_INTEGER;
            if(type)
                *type = GL_UNSIGNED_BYTE;
            break;

        case GL_RGB8_SNORM:
        case GL_RGBA8_SNORM:
            if(type)
                *type = GL_BYTE;
            break;

        default:
            if (*internal_format == GL_RGB8) {
                if (type && *type != GL_UNSIGNED_BYTE)
                    *type = GL_UNSIGNED_BYTE;
                if (format)
                    *format = GL_RGB;
            }
            else if (type && *internal_format == GL_RGBA16_SNORM && *type != GL_SHORT) {
                *type = GL_SHORT; 
            }
            break;
    }
}

void glTexParameterf(GLenum target, GLenum pname, GLfloat param) {
    LOG();
    pname = pname_convert(pname);
    LOG_D("glTexParameterf, target: %d, pname: %d, param: %f",target, pname, param);

    if (pname == GL_TEXTURE_LOD_BIAS_QCOM && !g_gles_caps.GL_QCOM_texture_lod_bias) {
        LOG_D("Does not support GL_QCOM_texture_lod_bias, skipped!")
        return;
    }

    LOAD_GLES(glTexParameterf, void, GLenum target, GLenum pname, GLfloat param);
    gles_glTexParameterf(target,pname, param);
    CHECK_GL_ERROR
}

void glTexImage1D(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid* pixels) {
    LOG();
    LOG_D("glTexImage1D not implemeted!");
    LOG_D("glTexImage1D, target: %d, level: %d, internalFormat: %d, width: %d, border: %d, format: %d, type: %d",
          target, level, internalFormat, width, border, format, type);
    return;
    internal_convert(reinterpret_cast<GLenum *>(&internalFormat), & type, &format);

    GLenum rtarget = map_tex_target(target);
    if (rtarget == GL_PROXY_TEXTURE_1D) {
        int max1 = 4096;
        LOAD_GLES(glGetIntegerv, void, GLenum pname, GLint * data)
        gles_glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max1);
        set_gl_state_proxy_width(((width << level) > max1) ? 0 : width);
        set_gl_state_proxy_intformat(internalFormat);
        return;
    }

//    LOAD_GLES(glTexImage1D, void, GLenum target, GLint level, GLint internalFormat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid* pixels);
//    gles_glTexImage1D(target, level, internalFormat, width, border, format, type, pixels);

    CHECK_GL_ERROR
}

void glTexImage2D(GLenum target, GLint level,GLint internalFormat,GLsizei width, GLsizei height,GLint border, GLenum format, GLenum type,const GLvoid* pixels) {
    LOG();
    auto& tex = g_textures[bound_texture];
    tex.internal_format = internalFormat;
    tex.format = format;
    LOG_D("mg_glTexImage2D,target: 0x%x,level: %d,internalFormat: 0x%x->0x%x,width: %d,height: %d,border: %d,format: 0x%x,type: 0x%x",target,level,internalFormat,internalFormat,width,height,border,format,type);
    internal_convert(reinterpret_cast<GLenum *>(&internalFormat), &type, &format);
    LOG_D("gles_glTexImage2D,target: 0x%x,level: %d,internalFormat: 0x%x->0x%x,width: %d,height: %d,border: %d,format: 0x%x,type: 0x%x",target,level,internalFormat,internalFormat,width,height,border,format,type);
    GLenum rtarget = map_tex_target(target);
    if(rtarget == GL_PROXY_TEXTURE_2D) {
        int max1 = 4096;
        LOAD_GLES(glGetIntegerv, void, GLenum pname, GLint * data)
        gles_glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max1);
        set_gl_state_proxy_width(((width<<level)>max1)?0:width);
        set_gl_state_proxy_height(((height<<level)>max1)?0:height);
        set_gl_state_proxy_intformat(internalFormat);
        return;
    }
    LOAD_GLES(glTexImage2D, void, GLenum target, GLint level,GLint internalFormat,GLsizei width, GLsizei height,GLint border, GLenum format, GLenum type,const GLvoid* pixels);
    gles_glTexImage2D(target, level, internalFormat, width, height, border, format, type, pixels);
    if (tex.format == GL_BGRA && internalFormat == GL_RGBA8 && width <= 128 && height <= 128) {  // xaero has 64x64 tiles...hack here
        LOG_D("Detected GL_BGRA format @ tex = %d, do swizzle", bound_texture);
        if (tex.swizzle_param[0] == 0) { // assert this as never called glTexParameteri(..., GL_TEXTURE_SWIZZLE_R, ...)
            tex.swizzle_param[0] = GL_RED;
            tex.swizzle_param[1] = GL_GREEN;
            tex.swizzle_param[2] = GL_BLUE;
            tex.swizzle_param[3] = GL_ALPHA;
        }

        GLint r = tex.swizzle_param[0];
        GLint g = tex.swizzle_param[1];
        GLint b = tex.swizzle_param[2];
        GLint a = tex.swizzle_param[3];
        tex.swizzle_param[0] = g;
        tex.swizzle_param[1] = b;
        tex.swizzle_param[2] = a;
        tex.swizzle_param[3] = r;
        tex.format = GL_RGBA;

        LOAD_GLES_FUNC(glTexParameteri);
        gles_glTexParameteri(target, GL_TEXTURE_SWIZZLE_R, tex.swizzle_param[0]);
        gles_glTexParameteri(target, GL_TEXTURE_SWIZZLE_G, tex.swizzle_param[1]);
        gles_glTexParameteri(target, GL_TEXTURE_SWIZZLE_B, tex.swizzle_param[2]);
        gles_glTexParameteri(target, GL_TEXTURE_SWIZZLE_A, tex.swizzle_param[3]);
        CHECK_GL_ERROR
    }

    CHECK_GL_ERROR
}

void glTexImage3D(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* pixels) {
    LOG();
    LOG_D("glTexImage3D, target: 0x%x, level: %d, internalFormat: 0x%x, width: 0x%x, height: %d, depth: %d, border: %d, format: 0x%x, type: %d",
          target, level, internalFormat, width, height, depth, border, format, type);

    internal_convert(reinterpret_cast<GLenum *>(&internalFormat), &type, &format);

    GLenum rtarget = map_tex_target(target);
    if (rtarget == GL_PROXY_TEXTURE_3D) {
        int max1 = 4096;
        LOAD_GLES(glGetIntegerv, void, GLenum pname, GLint * data)
        gles_glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max1);
        set_gl_state_proxy_width(((width << level) > max1) ? 0 : width);
        set_gl_state_proxy_height(((height << level) > max1) ? 0 : height);
        //set_gl_state_proxy_depth(((depth << level) > max1) ? 0 : depth);
        set_gl_state_proxy_intformat(internalFormat);
        return;
    }

    LOAD_GLES(glTexImage3D, void, GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* pixels);
    gles_glTexImage3D(target, level, internalFormat, width, height, depth, border, format, type, pixels);

    CHECK_GL_ERROR
}

void glTexStorage1D(GLenum target, GLsizei levels, GLenum internalFormat, GLsizei width) {
    LOG();
    LOG_D("glTexStorage1D not implemented!")
    LOG_D("glTexStorage1D, target: %d, levels: %d, internalFormat: %d, width: %d",
          target, levels, internalFormat, width);
    return;
    internal_convert(&internalFormat,NULL,NULL);

//    LOAD_GLES(glTexStorage1D, void, GLenum target, GLsizei levels, GLenum internalFormat, GLsizei width);
//    gles_glTexStorage1D(target, levels, internalFormat, width);

    CHECK_GL_ERROR
}

void glTexStorage2D(GLenum target, GLsizei levels, GLenum internalFormat, GLsizei width, GLsizei height) {
    LOG();
    LOG_D("glTexStorage2D, target: %d, levels: %d, internalFormat: %d, width: %d, height: %d",
          target, levels, internalFormat, width, height);

    internal_convert(&internalFormat,NULL,NULL);

    LOAD_GLES(glTexStorage2D, void, GLenum target, GLsizei levels, GLenum internalFormat, GLsizei width, GLsizei height);
    gles_glTexStorage2D(target, levels, internalFormat, width, height);

    LOAD_GLES(glGetError, GLenum);
    GLenum ERR = gles_glGetError();
    if (ERR != GL_NO_ERROR)
        LOG_E("glTexStorage2D ERROR: %d", ERR);
}

void glTexStorage3D(GLenum target, GLsizei levels, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth) {
    LOG();
    LOG_D("glTexStorage3D, target: %d, levels: %d, internalFormat: %d, width: %d, height: %d, depth: %d",
          target, levels, internalFormat, width, height, depth);

    internal_convert(&internalFormat,NULL,NULL);

    LOAD_GLES(glTexStorage3D, void, GLenum target, GLsizei levels, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth);
    gles_glTexStorage3D(target, levels, internalFormat, width, height, depth);

    CHECK_GL_ERROR
}

void glCopyTexImage1D(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border) {
    LOG();
    LOG_D("glCopyTexImage1D not implemented!");
    LOG_D("glCopyTexImage1D, target: %d, level: %d, internalFormat: %d, x: %d, y: %d, width: %d, border: %d",
          target, level, internalFormat, x, y, width, border);
    return;

//    LOAD_GLES(glCopyTexImage1D, void, GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border);
//    gles_glCopyTexImage1D(target, level, internalFormat, x, y, width, border);

    CHECK_GL_ERROR
}

static int is_depth_format(GLenum format) {
    switch(format) {
        case GL_DEPTH_COMPONENT:
        case GL_DEPTH_COMPONENT16:
        case GL_DEPTH_COMPONENT24:
        case GL_DEPTH_COMPONENT32F:
            return 1;
        default:
            return 0;
    }
}

static GLenum get_binding_for_target(GLenum target) {
    switch(target) {
        case GL_TEXTURE_2D: return GL_TEXTURE_BINDING_2D;
        case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
            return GL_TEXTURE_BINDING_CUBE_MAP;
        default: return 0;
    }
}

void glCopyTexImage2D(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border) {
    LOG();

    INIT_CHECK_GL_ERROR

    LOAD_GLES_FUNC(glGetTexLevelParameteriv);
    GLint realInternalFormat;
    gles_glGetTexLevelParameteriv(target, level, GL_TEXTURE_INTERNAL_FORMAT, &realInternalFormat);
    internalFormat = (GLenum)realInternalFormat;

    LOG_D("glCopyTexImage2D, target: %d, level: %d, internalFormat: %d, x: %d, y: %d, width: %d, height: %d, border: %d",
          target, level, internalFormat, x, y, width, height, border);

    if (is_depth_format(internalFormat)) {
        GLenum format = GL_DEPTH_COMPONENT;
        GLenum type = GL_UNSIGNED_INT;
        internal_convert(&internalFormat, &type, &format);
        LOAD_GLES(glTexImage2D, void, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
        gles_glTexImage2D(target, level, internalFormat, width, height, border, format, type, NULL);
        CHECK_GL_ERROR_NO_INIT
//        GLint prevReadFBO, prevDrawFBO;
        GLint prevDrawFBO;
//        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &prevReadFBO);
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &prevDrawFBO);
        CHECK_GL_ERROR_NO_INIT
        GLuint tempDrawFBO;
        glGenFramebuffers(1, &tempDrawFBO);
        CHECK_GL_ERROR_NO_INIT
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, tempDrawFBO);
        CHECK_GL_ERROR_NO_INIT
        GLint currentTex;
        glGetIntegerv(get_binding_for_target(target), &currentTex);
        CHECK_GL_ERROR_NO_INIT
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, target, currentTex, level);
        CHECK_GL_ERROR_NO_INIT

        if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            CHECK_GL_ERROR_NO_INIT
            glDeleteFramebuffers(1, &tempDrawFBO);
            CHECK_GL_ERROR_NO_INIT
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, prevDrawFBO);
            CHECK_GL_ERROR_NO_INIT
            return;
        }
        CHECK_GL_ERROR_NO_INIT

        LOAD_GLES(glBlitFramebuffer, void, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
        gles_glBlitFramebuffer(x, y, x + width, y + height,
                               0, 0, width, height,
                               GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        CHECK_GL_ERROR_NO_INIT

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, prevDrawFBO);
        CHECK_GL_ERROR_NO_INIT
        glDeleteFramebuffers(1, &tempDrawFBO);
        CHECK_GL_ERROR_NO_INIT
    } else {
        LOAD_GLES(glCopyTexImage2D, void, GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
        gles_glCopyTexImage2D(target, level, internalFormat, x, y, width, height, border);
        CHECK_GL_ERROR_NO_INIT
    }

    CHECK_GL_ERROR_NO_INIT
}

void glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) {
    LOG();
    GLint internalFormat;
    LOAD_GLES_FUNC(glGetTexLevelParameteriv);
    gles_glGetTexLevelParameteriv(target, level, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat);

    LOG_D("glCopyTexSubImage2D, target: %d, level: %d, ......, internalFormat: %d", target, level, internalFormat);

    if (is_depth_format((GLenum)internalFormat)) {
        GLint prevReadFBO, prevDrawFBO;
        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &prevReadFBO);
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &prevDrawFBO);

        GLuint tempDrawFBO;
        glGenFramebuffers(1, &tempDrawFBO);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, tempDrawFBO);

        GLint currentTex;
        glGetIntegerv(get_binding_for_target(target), &currentTex);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, target, currentTex, level);

        if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            glDeleteFramebuffers(1, &tempDrawFBO);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, prevDrawFBO);
            return;
        }

        LOAD_GLES(glBlitFramebuffer, void, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
        gles_glBlitFramebuffer(x, y, x + width, y + height,
                               xoffset, yoffset, xoffset + width, yoffset + height,
                               GL_DEPTH_BUFFER_BIT, GL_NEAREST);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, prevDrawFBO);
        glDeleteFramebuffers(1, &tempDrawFBO);
    } else {
        LOAD_GLES(glCopyTexSubImage2D, void, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
        gles_glCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
    }

    CHECK_GL_ERROR
}

void glRenderbufferStorage(GLenum target, GLenum internalFormat, GLsizei width, GLsizei height) {
    LOG();

    INIT_CHECK_GL_ERROR_FORCE

    CLEAR_GL_ERROR_NO_INIT

    LOAD_GLES_FUNC(glGetTexLevelParameteriv);
    GLint realInternalFormat;
    gles_glGetTexLevelParameteriv(target, 0, GL_TEXTURE_INTERNAL_FORMAT, &realInternalFormat);
    ERR = gles_glGetError();
    if (realInternalFormat != 0 && ERR == GL_NO_ERROR)
        internalFormat = (GLenum)realInternalFormat;
    else
        internalFormat = GL_DEPTH_COMPONENT24;

    CLEAR_GL_ERROR_NO_INIT

    LOG_D("glRenderbufferStorage, target: 0x%x, internalFormat: 0x%x, width: %d, height: %d",
          target, internalFormat, width, height);

    LOAD_GLES(glRenderbufferStorage, void, GLenum target, GLenum internalFormat, GLsizei width, GLsizei height);
    gles_glRenderbufferStorage(target, internalFormat, width, height);

    CHECK_GL_ERROR_NO_INIT
}

void glRenderbufferStorageMultisample(GLenum target, GLsizei samples, GLenum internalFormat, GLsizei width, GLsizei height) {
    LOG();

    INIT_CHECK_GL_ERROR_FORCE

    CLEAR_GL_ERROR_NO_INIT

    LOAD_GLES_FUNC(glGetTexLevelParameteriv);
    GLint realInternalFormat;
    gles_glGetTexLevelParameteriv(target, 0, GL_TEXTURE_INTERNAL_FORMAT, &realInternalFormat);
    ERR = gles_glGetError();
    if (realInternalFormat != 0 && ERR == GL_NO_ERROR)
        internalFormat = (GLenum)realInternalFormat;
    else
        internalFormat = GL_DEPTH_COMPONENT24;


    LOG_D("glRenderbufferStorageMultisample, target: %d, samples: %d, internalFormat: %d, width: %d, height: %d",
          target, samples, internalFormat, width, height);

    LOAD_GLES(glRenderbufferStorageMultisample, void, GLenum target, GLsizei samples, GLenum internalFormat, GLsizei width, GLsizei height);
    gles_glRenderbufferStorageMultisample(target, samples, internalFormat, width, height);

    CHECK_GL_ERROR_NO_INIT
}

void glGetTexLevelParameterfv(GLenum target, GLint level,GLenum pname, GLfloat *params) {
    LOG();
    LOG_D("glGetTexLevelParameterfv,target: %d, level: %d, pname: %d",target,level,pname);
    GLenum rtarget = map_tex_target(target);
    if (rtarget==GL_PROXY_TEXTURE_2D) {
        switch (pname) {
            case GL_TEXTURE_WIDTH:
                (*params) = nlevel(gl_state->proxy_width, level);
                break;
            case GL_TEXTURE_HEIGHT:
                (*params) = nlevel(gl_state->proxy_height, level);
                break;
            case GL_TEXTURE_INTERNAL_FORMAT:
                (*params) = gl_state->proxy_intformat;
                break;
        }
        return;
    }
    LOAD_GLES(glGetTexLevelParameterfv, void, GLenum target, GLint level,GLenum pname, GLfloat *params);
    gles_glGetTexLevelParameterfv(target,level,pname,params);
    CHECK_GL_ERROR
}

void glGetTexLevelParameteriv(GLenum target, GLint level,GLenum pname, GLint *params) {
    LOG();
    LOG_D("glGetTexLevelParameteriv,target: %d, level: %d, pname: %d",target,level,pname);
    GLenum rtarget = map_tex_target(target);
    if (rtarget==GL_PROXY_TEXTURE_2D) {
        switch (pname) {
            case GL_TEXTURE_WIDTH:
                (*params) = nlevel(gl_state->proxy_width, level);
                break;
            case GL_TEXTURE_HEIGHT:
                (*params) = nlevel(gl_state->proxy_height, level);
                break;
            case GL_TEXTURE_INTERNAL_FORMAT:
                (*params) = gl_state->proxy_intformat;
                break;
        }
        return;
    }
    LOAD_GLES(glGetTexLevelParameteriv, void, GLenum target, GLint level,GLenum pname, GLint *params);
    GLint *fparams = NULL;
    gles_glGetTexLevelParameteriv(target,level,pname,params);
    CHECK_GL_ERROR
}

void glTexParameteriv(GLenum target, GLenum pname, const GLint* params) {
    LOG_D("glTexParameteriv, target: %d, pname: %d, params[0]: %d",
          target, pname, params ? params[0] : 0);
    LOAD_GLES_FUNC(glTexParameteriv);
    LOAD_GLES_FUNC(glTexParameteri);

    if (pname == GL_TEXTURE_SWIZZLE_RGBA) {
        LOG_D("find GL_TEXTURE_SWIZZLE_RGBA, now use glTexParameteri");
        if (params) {
            // deferred those call to draw call?
            gles_glTexParameteri(target, GL_TEXTURE_SWIZZLE_R, params[0]);
            gles_glTexParameteri(target, GL_TEXTURE_SWIZZLE_G, params[1]);
            gles_glTexParameteri(target, GL_TEXTURE_SWIZZLE_B, params[2]);
            gles_glTexParameteri(target, GL_TEXTURE_SWIZZLE_A, params[3]);

            // save states for now
            texture_t& tex = g_textures[bound_texture];
            tex.swizzle_param[0] = params[0];
            tex.swizzle_param[1] = params[1];
            tex.swizzle_param[2] = params[2];
            tex.swizzle_param[3] = params[3];
        } else {
            LOG_E("glTexParameteriv: params is null for GL_TEXTURE_SWIZZLE_RGBA");
        }
    } else {
        gles_glTexParameteriv(target, pname, params);
    }

    CHECK_GL_ERROR
}

void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels) {
    LOG();
    LOAD_GLES_FUNC(glTexSubImage2D)
    gles_glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);

    CLEAR_GL_ERROR
}

void glBindTexture(GLenum target, GLuint texture) {
    LOG();
    LOG_D("glBindTexture(0x%x, %d)", target, texture);
    LOAD_GLES_FUNC(glBindTexture)
    INIT_CHECK_GL_ERROR
    gles_glBindTexture(target, texture);
    CHECK_GL_ERROR_NO_INIT

    if (target == GL_TEXTURE_2D) { // only care about 2D textures for now
        g_textures[texture] = {
                .target = target,
                .texture = texture,
                .format = 0,
                .swizzle_param = {0}
        };
        bound_texture = texture;
    }
}

void glDeleteTextures(GLsizei n, const GLuint *textures) {
    LOG();
    LOAD_GLES_FUNC(glDeleteTextures)
    INIT_CHECK_GL_ERROR
    gles_glDeleteTextures(n, textures);
    CHECK_GL_ERROR_NO_INIT

    for (GLsizei i = 0; i < n; ++i) {
        g_textures.erase(textures[i]);
        if (bound_texture == textures[i])
            bound_texture = 0;
    }
}

void glGenerateTextureMipmap(GLuint texture) {
    GLint currentTexture;
    auto& tex = g_textures[bound_texture];
    GLenum target = tex.target;
    GLenum binding = get_binding_for_target(target);
    if (binding == 0) return;
    glGetIntegerv(binding, &currentTexture);
    glBindTexture(target, texture);
    glGenerateMipmap(target);
    glBindTexture(target, currentTexture);
}

void glGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, void* pixels) {
    GLint prevFBO;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFBO);
    GLenum bindingTarget = get_binding_for_target(target);
    if (bindingTarget == 0) return;
    GLint oldTexBinding;
    glActiveTexture(GL_TEXTURE0);
    glGetIntegerv(bindingTarget, &oldTexBinding);
    GLuint texture = static_cast<GLuint>(oldTexBinding);
    if (texture == 0) return;
    GLint width, height;
    glBindTexture(target, texture);
    glGetTexLevelParameteriv(target, level, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(target, level, GL_TEXTURE_HEIGHT, &height);
    glBindTexture(target, oldTexBinding);
    if (width <= 0 || height <= 0) return;
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    if (target == GL_TEXTURE_2D || (target >= GL_TEXTURE_CUBE_MAP_POSITIVE_X && target <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z)) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target, texture, level);
    } else {
        glDeleteFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, prevFBO);
        return;
    }
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        glDeleteFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, prevFBO);
        return;
    }
    GLint oldViewport[4];
    glGetIntegerv(GL_VIEWPORT, oldViewport);
    glViewport(0, 0, width, height);
    GLint oldPackAlignment;
    glGetIntegerv(GL_PACK_ALIGNMENT, &oldPackAlignment);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glReadPixels(0, 0, width, height, format, type, pixels);
    glPixelStorei(GL_PACK_ALIGNMENT, oldPackAlignment);
    glViewport(oldViewport[0], oldViewport[1], oldViewport[2], oldViewport[3]);
    glDeleteFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, prevFBO);
}

void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels) {
    LOG()
    LOAD_GLES_FUNC(glReadPixels)
    LOG_D("glReadPixels, x=%d, y=%d, width=%d, height=%d, format=0x%x, type=0x%x, pixels=0x%x",
          x, y, width, height, format, type, pixels)
    if (format == GL_BGRA && type == GL_UNSIGNED_INT_8_8_8_8) {
        format = GL_RGBA;
        type = GL_UNSIGNED_BYTE;
    }
    LOG_D("glReadPixels converted, x=%d, y=%d, width=%d, height=%d, format=0x%x, type=0x%x, pixels=0x%x",
          x, y, width, height, format, type, pixels)
    gles_glReadPixels(x, y, width, height, format, type, pixels);
    CHECK_GL_ERROR
}

void glTexParameteri(GLenum target, GLenum pname, GLint param) {
    LOG()
    pname = pname_convert(pname);
    LOG_D("glTexParameterfv, pname: %d", pname)

    if (pname == GL_TEXTURE_LOD_BIAS_QCOM && !g_gles_caps.GL_QCOM_texture_lod_bias) {
        LOG_D("Does not support GL_QCOM_texture_lod_bias, skipped!")
        return;
    }

    LOAD_GLES_FUNC(glTexParameteri)
    gles_glTexParameteri(target,pname,param);
    CHECK_GL_ERROR
}