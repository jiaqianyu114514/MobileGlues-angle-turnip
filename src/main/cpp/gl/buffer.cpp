//
// Created by BZLZHH on 2025/1/28.
//

#include "buffer.h"
#include <unordered_map>

#define DEBUG 0

std::unordered_map<GLuint, BufferMapping> g_active_mappings;

static GLenum get_binding_query(GLenum target) {
    switch(target) {
        case GL_ARRAY_BUFFER:          return GL_ARRAY_BUFFER_BINDING;
        case GL_ELEMENT_ARRAY_BUFFER:  return GL_ELEMENT_ARRAY_BUFFER_BINDING;
        case GL_PIXEL_PACK_BUFFER:     return GL_PIXEL_PACK_BUFFER_BINDING;
        case GL_PIXEL_UNPACK_BUFFER:   return GL_PIXEL_UNPACK_BUFFER_BINDING;
        default:                       return 0;
    }
}

void* glMapBuffer(GLenum target, GLenum access) {
    LOG()
    if (get_binding_query(target) == 0) {
        return NULL;
    }
    GLint current_buffer;
    glGetIntegerv(get_binding_query(target), &current_buffer);
    if (current_buffer == 0) {
        return NULL;
    }
    if (g_active_mapping.mapped_ptr != NULL) {
        return NULL;
    }
    GLint buffer_size;
    glGetBufferParameteriv(target, GL_BUFFER_SIZE, &buffer_size);
    if (buffer_size <= 0 || glGetError() != GL_NO_ERROR) {
        return NULL;
    }
    GLbitfield flags = 0;
    switch (access) {
        case GL_READ_ONLY:  flags = GL_MAP_READ_BIT; break;
        case GL_WRITE_ONLY: flags = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT; break;
        case GL_READ_WRITE: flags = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT; break;
        default:  
            return NULL;
    }
    void* ptr = glMapBufferRange(target, 0, buffer_size, flags);
    if (!ptr) return NULL;
    BufferMapping mapping;
    mapping.target = target;
    mapping.buffer_id = (GLuint)current_buffer;
    mapping.mapped_ptr = ptr;
    mapping.size = buffer_size;
    mapping.flags = flags;
    mapping.is_dirty = (flags & GL_MAP_WRITE_BIT) ? GL_TRUE : GL_FALSE;
    g_active_mappings[current_buffer] = mapping;
    CHECK_GL_ERROR
    return ptr;
}

GLboolean force_unmap() {
    if (g_active_mappings.empty())
        return GL_FALSE;

    LOAD_GLES(glBindBuffer, void, GLenum target, GLuint buffer)
    LOAD_GLES(glUnmapBuffer, GLboolean, GLenum target);

    for (auto& [buffer, binding]: g_active_mappings) {
        GLint prev_buffer = 0;
        GLenum binding_query = get_binding_query(binding.target);
        glGetIntegerv(binding_query, &prev_buffer);

        gles_glBindBuffer(binding.target, binding.buffer_id);
        GLboolean result = gles_glUnmapBuffer(binding.target);
        gles_glBindBuffer(binding.target, prev_buffer);
    }

    g_active_mappings.clear();

    return GL_TRUE;
}

GLboolean glUnmapBuffer(GLenum target) {
    LOG()

    GLint buffer;
    GLenum binding_query = get_binding_query(target);
    glGetIntegerv(binding_query, &buffer);

    if (buffer == 0)
        return GL_FALSE;

    LOAD_GLES(glUnmapBuffer, GLboolean, GLenum target);
    GLboolean result = gles_glUnmapBuffer(target);

    g_active_mappings.erase(buffer);

    CHECK_GL_ERROR
    return result;
}

void glBufferStorage(GLenum target, GLsizeiptr size, const void* data, GLbitfield flags) {
    LOG()
    LOAD_GLES_FUNC(glBufferStorageEXT)
    gles_glBufferStorageEXT(target,size,data,flags);
    CHECK_GL_ERROR
}