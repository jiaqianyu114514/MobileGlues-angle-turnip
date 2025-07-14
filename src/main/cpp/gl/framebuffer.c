//
// Created by hanji on 2025/2/6.
//

#include "framebuffer.h"
#include "log.h"
#include "../config/settings.h"

#define DEBUG 0

struct framebuffer_t* bound_framebuffer;

GLint MAX_DRAW_BUFFERS = 0;

GLint getMaxDrawBuffers() {
    if (!MAX_DRAW_BUFFERS) {
        LOAD_GLES(glGetIntegerv, void, GLenum pname, GLint *params)
        gles_glGetIntegerv(GL_MAX_DRAW_BUFFERS, &MAX_DRAW_BUFFERS);
    }
    return MAX_DRAW_BUFFERS;
}

void rebind_framebuffer(GLenum old_attachment, GLenum target_attachment) {
    if (!bound_framebuffer)
        return;

    struct attachment_t* attach;
    if (bound_framebuffer->current_target == GL_DRAW_FRAMEBUFFER)
        attach = bound_framebuffer->draw_attachment;
    else
        attach = bound_framebuffer->read_attachment;

    if (!attach)
        return;

    struct attachment_t attachment = attach[old_attachment - GL_COLOR_ATTACHMENT0];

    LOAD_GLES(glFramebufferTexture2D, void, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
    gles_glFramebufferTexture2D(bound_framebuffer->current_target, target_attachment, attachment.textarget, attachment.texture, attachment.level);
}

void glBindFramebuffer(GLenum target, GLuint framebuffer) {
    LOG()

    INIT_CHECK_GL_ERROR

    LOG_D("glBindFramebuffer(0x%x, %d)", target, framebuffer)

    LOAD_GLES(glBindFramebuffer, void, GLenum target, GLuint framebuffer)
    gles_glBindFramebuffer(target, framebuffer);
    CHECK_GL_ERROR_NO_INIT

    if (!bound_framebuffer) {
        bound_framebuffer = malloc(sizeof(struct framebuffer_t));
        memset(bound_framebuffer, 0, sizeof(struct framebuffer_t));
    }

    switch (target) {
        case GL_DRAW_FRAMEBUFFER:
            free(bound_framebuffer->draw_attachment);
            bound_framebuffer->draw_attachment = malloc(getMaxDrawBuffers() * sizeof(struct attachment_t));
            break;
        case GL_READ_FRAMEBUFFER:
            free(bound_framebuffer->read_attachment);
            bound_framebuffer->read_attachment = malloc(getMaxDrawBuffers() * sizeof(struct attachment_t));
            break;
        case GL_FRAMEBUFFER:
            free(bound_framebuffer->draw_attachment);
            bound_framebuffer->draw_attachment = malloc(getMaxDrawBuffers() * sizeof(struct attachment_t));
            free(bound_framebuffer->read_attachment);
            bound_framebuffer->read_attachment = malloc(getMaxDrawBuffers() * sizeof(struct attachment_t));
            break;
        default:
            break;
    }

    CHECK_GL_ERROR_NO_INIT
}

void glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) {
    LOG()

    LOG_D("glFramebufferTexture2D(0x%x, 0x%x, 0x%x, %d, %d)", target, attachment, textarget, texture, level)

    if (bound_framebuffer && attachment - GL_COLOR_ATTACHMENT0 <= getMaxDrawBuffers()) {
        struct attachment_t* attach;
        if (target == GL_DRAW_FRAMEBUFFER)
            attach = bound_framebuffer->draw_attachment;
        else
            attach = bound_framebuffer->read_attachment;

        if (attach) {
            attach[attachment - GL_COLOR_ATTACHMENT0].textarget = textarget;
            attach[attachment - GL_COLOR_ATTACHMENT0].texture = texture;
            attach[attachment - GL_COLOR_ATTACHMENT0].level = level;
        }

        bound_framebuffer->current_target = target;
    }

    LOAD_GLES(glFramebufferTexture2D, void, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
    gles_glFramebufferTexture2D(target, attachment, textarget, texture, level);

    CHECK_GL_ERROR
}

void glDrawBuffer(GLenum buffer) {
    LOG()

    LOAD_GLES(glDrawBuffers, void, GLsizei n, const GLenum *bufs)

    GLint currentFBO;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFBO);

    if (currentFBO == 0) { // 默认帧缓冲
        GLenum buffers[1] = {GL_NONE};
        switch (buffer) {
            case GL_FRONT:
            case GL_BACK:
            case GL_NONE:
                buffers[0] = buffer;
                gles_glDrawBuffers(1, buffers);
                break;
            default:
                // 生成错误：GL_INVALID_ENUM
                break;
        }
    } else { // FBO场景
        GLint maxAttachments;
        glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxAttachments);

        if (buffer == GL_NONE) {
            GLenum *buffers = (GLenum *)alloca(maxAttachments * sizeof(GLenum));
            for (int i = 0; i < maxAttachments; i++) {
                buffers[i] = GL_NONE;
            }
            gles_glDrawBuffers(maxAttachments, buffers);
        } else if (buffer >= GL_COLOR_ATTACHMENT0 &&
                   buffer < GL_COLOR_ATTACHMENT0 + maxAttachments) {
            GLenum *buffers = (GLenum *)alloca(maxAttachments * sizeof(GLenum));
            for (int i = 0; i < maxAttachments; i++) {
                buffers[i] = (i == (buffer - GL_COLOR_ATTACHMENT0)) ? buffer : GL_NONE;
            }
            gles_glDrawBuffers(maxAttachments, buffers);
        } else {
            // 生成错误：GL_INVALID_ENUM
        }
    }
}

void glDrawBuffers(GLsizei n, const GLenum *bufs) {
    LOG()

    LOG_D("glDrawBuffers(%d, %p), [0]=0x%x", n, bufs, n ? bufs[0] : 0)

    GLenum new_bufs[n];

    for (int i = 0; i < n; i++) {
        if (bufs[i] >= GL_COLOR_ATTACHMENT0 && bufs[i] <= GL_COLOR_ATTACHMENT0 + getMaxDrawBuffers()) {
            GLenum target_attachment = GL_COLOR_ATTACHMENT0 + i;
            new_bufs[i] = target_attachment;
            rebind_framebuffer(bufs[i], target_attachment);
        } else {
            new_bufs[i] = bufs[i];
        }
    }

    LOAD_GLES(glDrawBuffers, void, GLsizei n, const GLenum *bufs)
    gles_glDrawBuffers(n, new_bufs);

    CHECK_GL_ERROR
}

GLenum glCheckFramebufferStatus(GLenum target) {
    LOG()
    LOAD_GLES_FUNC(glCheckFramebufferStatus)
    GLenum status = gles_glCheckFramebufferStatus(target);
    if(global_settings.ignore_error >= 2 && status != GL_FRAMEBUFFER_COMPLETE) {
        LOG_W_FORCE("Framebuffer %d isn't GL_FRAMEBUFFER_COMPLETE: %d", target, status);
        LOG_W_FORCE("Now try to cheat.");
        return GL_FRAMEBUFFER_COMPLETE;
    }
    return status;
    CHECK_GL_ERROR
}