//
// Created by BZLZHH on 2025/1/29.
//

#include "drawing.h"
#include "buffer.h"
#include "framebuffer.h"

#define DEBUG 0

void glMultiDrawElementsBaseVertex( GLenum mode, GLsizei *counts, GLenum type, const void * const *indices, GLsizei primcount, const GLint * basevertex) {
    LOG();
    if (primcount <= 0 || !counts || !indices) return;
    for (int i = 0; i < primcount; i++) {
        if (counts[i] > 0)
            glDrawElementsBaseVertex(mode,
                    counts[i],
                    type,
                    indices[i],
                    basevertex[i]);
    }
}


void glMultiDrawElements(GLenum mode,const GLsizei * count,GLenum type,const void * const * indices,GLsizei primcount) {
    LOG();
    if (primcount <= 0 || !count || !indices) return;
    for (GLsizei i = 0; i < primcount; ++i) {
        if (count[i] > 0) {
            glDrawElements(
                    mode,
                    count[i],
                    type,
                    indices[i]
            );
        }
    }
}