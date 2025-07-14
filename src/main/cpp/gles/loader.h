#ifndef MOBILEGLUES_GLES_LOADER_H_
#define MOBILEGLUES_GLES_LOADER_H_

#include <stdbool.h>
#include "../gl/log.h"
#include "../gl/gl.h"
#include "gles.h"
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

void *proc_address(void *lib, const char *name);
extern void *gles, *egl;


#ifdef __cplusplus
}
#endif

void init_target_gles();

#define WARN_NULL(name) if (name == NULL) { LOG_W("%s line %d function %s: " #name " is NULL\n", __FILE__, __LINE__, __func__); }

#define WARN_GLES_NULL(name) \
    if (g_gles_func.name == NULL) { \
        LOG_W("%s line %d function %s: " #name " is NULL\n", __FILE__, __LINE__, __func__); \
    }

#if GLOBAL_DEBUG
#define INIT_GLES_FUNC(name)                                      \
    {                                                             \
        LOG_D("INIT_GLES_FUNC(%s)", #name);                       \
        g_gles_func.name = (name##_PTR)proc_address(gles, #name); \
        WARN_GLES_NULL(name)                                      \
    }
#else
#define INIT_GLES_FUNC(name) \
    {                        \
        g_gles_func.name = (name##_PTR)proc_address(gles, #name); \
    }
#endif

#define LOAD_GLES_FUNC(name) \
    name##_PTR gles_##name = g_gles_func.name;

#define LOAD_GLES(name,type, ...) LOAD_GLES_FUNC(name)
#define LOAD_GLES2(name,type, ...) LOAD_GLES_FUNC(name)
#define LOAD_GLES3(name,type, ...) LOAD_GLES_FUNC(name)

void *open_lib(const char **names, const char *override);
#define LOAD_RAW_GLES(name, type, ...)                  \
    gles_##name = g_gles_func.name;                     \
//#define LOAD_RAW_GLES(name, type, ...)                                      \
//    {                                                                       \
//        if (gles != NULL) {                                                 \
//            gles_##name = (name##_PTR)proc_address(gles, #name);            \
//        }                                                                   \
//        WARN_NULL(gles_##name);                                             \
//    }

#define LOAD_LIB(type, name, ...)                                           \
    typedef type (*name##_PTR)(__VA_ARGS__);                                \
    name##_PTR gles_##name = NULL;                                          \
    LOAD_RAW_GLES(name, type, __VA_ARGS__)

//#define LOAD_GLES(name,type, ...)            LOAD_LIB(type, name, __VA_ARGS__)
//#define LOAD_GLES2(name,type, ...)           LOAD_LIB(type, name, __VA_ARGS__)
//#define LOAD_GLES3(name,type, ...)           LOAD_LIB(type, name, __VA_ARGS__)
#define LOAD_EGL(name) \
static name##_PTR egl_##name = NULL; \
{ \
        static bool first = true; \
        if (first) { \
            first = false; \
            if (egl != NULL) { \
                egl_##name = (name##_PTR)proc_address(egl, #name); \
            } \
            WARN_NULL(egl_##name); \
        } \
}

#define CLEAR_GL_ERROR \
    LOAD_GLES(glGetError, GLenum)                                           \
    GLenum ERR = gles_glGetError();                                         \
    while (ERR != GL_NO_ERROR)                                              \
        ERR = gles_glGetError();

#define CLEAR_GL_ERROR_NO_INIT \
    ERR = gles_glGetError();                                                \
    while (ERR != GL_NO_ERROR)                                              \
        ERR = gles_glGetError();

#if GLOBAL_DEBUG
#define CHECK_GL_ERROR                                                      \
    LOAD_GLES(glGetError, GLenum)                                           \
    GLenum ERR = gles_glGetError();                                         \
    while (ERR != GL_NO_ERROR) {                                            \
        LOG_E("ERROR: %d @ %s:%d", ERR, __FILE__, __LINE__)                 \
        ERR = gles_glGetError();                                            \
    }

#define INIT_CHECK_GL_ERROR                                                 \
    LOAD_GLES(glGetError, GLenum)                                           \
    GLenum ERR = GL_NO_ERROR;

#define CHECK_GL_ERROR_NO_INIT \
    ERR = gles_glGetError();                                               \
    while (ERR != GL_NO_ERROR) {                                           \
        LOG_E("ERROR: %d @ %s:%d", ERR, __FILE__, __LINE__)                \
        ERR = gles_glGetError();                                           \
    }
#else
#define CHECK_GL_ERROR {}
#define INIT_CHECK_GL_ERROR  {}
#define CHECK_GL_ERROR_NO_INIT {}
#endif

#define INIT_CHECK_GL_ERROR_FORCE                                           \
    LOAD_GLES(glGetError, GLenum)                                           \
    GLenum ERR = GL_NO_ERROR;


#define NATIVE_FUNCTION_HEAD(type,name,...)                                 \
GLAPI GLAPIENTRY type name##ARB(__VA_ARGS__) __attribute__((alias(#name))); \
GLAPI GLAPIENTRY type name(__VA_ARGS__)  {                                  \
LOAD_GLES_FUNC(name)

#if GLOBAL_DEBUG
#define NATIVE_FUNCTION_END(type,name,...)                                  \
    LOG_D("Use native function: %s @ %s(...)", RENDERERNAME, __FUNCTION__); \
    LOAD_RAW_GLES(name, type, __VA_ARGS__);                                 \
    type ret = gles_##name(__VA_ARGS__);                                    \
    LOAD_GLES(glGetError, GLenum)                                           \
    GLenum ERR = gles_glGetError();                                         \
    if (ERR != GL_NO_ERROR)                                                 \
        LOG_E("ERROR: %d", ERR)                                             \
    return ret;                                                             \
}
#else
#define NATIVE_FUNCTION_END(type,name,...)                                  \
    LOG_D("Use native function: %s @ %s(...)", RENDERERNAME, __FUNCTION__); \
    LOAD_RAW_GLES(name, type, __VA_ARGS__);                                 \
    type ret = gles_##name(__VA_ARGS__);                                    \
    CHECK_GL_ERROR                                                          \
    return ret;                                                             \
}
#endif

#if GLOBAL_DEBUG
#define NATIVE_FUNCTION_END_NO_RETURN(type,name,...)                        \
    LOG_D("Use native function: %s @ %s(...)", RENDERERNAME, __FUNCTION__); \
    LOAD_RAW_GLES(name, type, __VA_ARGS__);                                 \
    gles_##name(__VA_ARGS__);                                               \
    CHECK_GL_ERROR                                                          \
}
#else
#define NATIVE_FUNCTION_END_NO_RETURN(type,name,...)                        \
    LOG_D("Use native function: %s @ %s(...)", RENDERERNAME, __FUNCTION__); \
    LOAD_RAW_GLES(name, type, __VA_ARGS__);                                 \
    gles_##name(__VA_ARGS__);                                               \
}
#endif

#define STUB_FUNCTION_HEAD(type,name,...)                                   \
GLAPI GLAPIENTRY type name(__VA_ARGS__) {

#define STUB_FUNCTION_END(type,name,...)                                    \
    LOG_W("No function: %s @ %s(...)", RENDERERNAME, __FUNCTION__);         \
    return (type)0;                                                         \
}

#define STUB_FUNCTION_END_NO_RETURN(type,name,...)                          \
    LOG_W("No function: %s @ %s(...)", RENDERERNAME, __FUNCTION__);         \
}

struct gles_caps_t {
    int GL_EXT_buffer_storage;
    int GL_EXT_disjoint_timer_query;
    int GL_QCOM_texture_lod_bias;
};

extern struct gles_caps_t g_gles_caps;

#endif // MOBILEGLUES_GLES_LOADER_H_
