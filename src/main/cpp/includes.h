//
// Created by Swung 0x48 on 2024/10/7.
//

#ifndef MOBILEGLUES_INCLUDES_H
#define MOBILEGLUES_INCLUDES_H

#define RENDERERNAME "MobileGlues"
#include <android/log.h>
#include <dlfcn.h>

#include <EGL/egl.h>
#include <GLES3/gl32.h>

#include "egl/egl.h"
#include "egl/loader.h"

#define _mglues_dlopen(name) dlopen(name, RTLD_LAZY)
#define _mglues_dlclose(handle) dlclose(handle)
#define _mglues_dlsym(handle, name) dlsym(handle, name)

static int g_initialized = 0;

void proc_init();

EGLContext mglues_eglCreateContext (EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list);
EGLBoolean mglues_eglDestroyContext(EGLDisplay dpy, EGLContext ctx);
EGLBoolean mglues_eglMakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx);


#endif //MOBILEGLUES_INCLUDES_H
