//
// Created by Swung 0x48 on 2024/10/7.
//

#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include "includes.h"
#include "gl/gl.h"
#include "egl/egl.h"
#include "egl/loader.h"
#include "gles/loader.h"
#include "gl/envvars.h"
#include "gl/log.h"
#include "config/settings.h"

#define DEBUG 0

#ifdef __cplusplus
extern "C" {
#endif

__attribute__((used)) const char* copyright = "Copyright (C) 2025 Swung0x48, BZLZHH, Tungsten. All rights reserved. Logo artwork kindly provided by Aou156.";

extern char* (*MesaConvertShader)(const char *src, unsigned int type, unsigned int glsl, unsigned int essl);
void init_libshaderconv() {
    const char *shaderconv_lib = "libshaderconv";
    const char *func_name = "MesaConvertShader";
    const char *glslconv_name[] = {shaderconv_lib, NULL};
    void* glslconv = open_lib(glslconv_name, shaderconv_lib);
    if (glslconv == NULL) {
        LOG_D("%s not found\n", shaderconv_lib);
    }
    else {
        MesaConvertShader = dlsym(glslconv, func_name);
        if (MesaConvertShader) {
            LOG_D("%s loaded\n", shaderconv_lib);
        } else {
            LOG_D("failed to load %s\n", shaderconv_lib);
        }
    }
}

void init_config() {
    if(mkdir(MG_DIRECTORY_PATH, 0755) != 0 && errno != EEXIST) {
        LOG_E("Error creating MG directory.\n")
        return;
    }
    config_refresh();
}

void show_copyright() {
    LOG_V("MobileGlues Copyright: ");
    LOG_V("  %s", copyright);
}

void load_libs();
void proc_init() {
    init_config();

    clear_log();
    start_log();

    init_settings();

    LOG_V("Initializing %s ...", RENDERERNAME);
    show_copyright();

    load_libs();
    init_target_egl();
    init_target_gles();

    init_libshaderconv();

    g_initialized = 1;
}

#ifdef __cplusplus
}
#endif
