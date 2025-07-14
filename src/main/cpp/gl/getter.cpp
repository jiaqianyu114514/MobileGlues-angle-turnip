//
// Created by BZLZHH on 2025/1/28.
//

#include "getter.h"
#include "../config/settings.h"
#include <string>

#define DEBUG 0

void glGetIntegerv(GLenum pname, GLint *params) {
    LOG();
    LOG_D("glGetIntegerv, pname: 0x%x",pname);
    if (pname == GL_CONTEXT_PROFILE_MASK) {
        (*params) = GL_CONTEXT_CORE_PROFILE_BIT;
        return;
    } 
    if (pname == GL_NUM_EXTENSIONS) {
        static GLint num_extensions = -1;
        if (num_extensions == -1) {
            const GLubyte* ext_str = glGetString(GL_EXTENSIONS);
            if (ext_str) {
                char* copy = strdup((const char*)ext_str);
                char* token = strtok(copy, " ");
                num_extensions = 0;
                while (token) {
                    num_extensions++;
                    token = strtok(nullptr, " ");
                }
                free(copy);
            } else {
                num_extensions = 0;
            }
        }
        (*params) = num_extensions;
        return;
    }
    LOAD_GLES_FUNC(glGetIntegerv);
    gles_glGetIntegerv(pname, params);
    LOG_D("  -> %d",*params);
    CHECK_GL_ERROR
}

GLenum glGetError() {
    LOG();
    LOAD_GLES_FUNC(glGetError);
    GLuint err = gles_glGetError();
    if (err != GL_NO_ERROR) {
        if(global_settings.ignore_error >= 2) {
            // no logging without DEBUG
            LOG_W("glGetError\n -> %d", err)
            LOG_W("Now try to cheat.")
            return GL_NO_ERROR;
        } else {
            LOG_E(" -> %d", err)
        }
    }
    return err;
}

static std::string es_ext;
const char* GetExtensionsList() {
    return es_ext.c_str();
}

void InitGLESBaseExtensions() {
    es_ext = "GL_ARB_fragment_program "
             "GL_ARB_vertex_buffer_object "
             "GL_ARB_vertex_array_object "
             "GL_ARB_vertex_buffer "
             "GL_EXT_vertex_array "
             "GL_ARB_ES2_compatibility "
             "GL_ARB_ES3_compatibility "
             "GL_EXT_packed_depth_stencil "
             "GL_EXT_depth_texture "
             "GL_ARB_depth_texture "
             "GL_ARB_shading_language_100 "
             "ARB_imaging "
             "GL_ARB_draw_buffers_blend "
             "OpenGL15 "
             "OpenGL30 "
             "OpenGL31 "
             "OpenGL32 "
             "OpenGL33 "
             "OpenGL40 "
             "GL_ARB_shader_storage_buffer_object "
             "GL_ARB_shader_image_load_store "
             "GL_ARB_get_program_binary ";
}

void AppendExtension(const char* ext) {
    es_ext += ext;
    es_ext += ' ';
}

const char* getBeforeThirdSpace(const char* str) {
    static char buffer[256];
    int spaceCount = 0;
    const char* start = str;
    while (*str) {
        if (*str == ' ') {
            spaceCount++;
            if (spaceCount == 3) break;
        }
        str++;
    }
    int len = str - start;
    if (len >= sizeof(buffer)) len = sizeof(buffer) - 1;
    strncpy(buffer, start, len);
    buffer[len] = '\0';
    return buffer;
}


const char* getGpuName() {
    LOAD_GLES_FUNC(glGetString);
    const char *gpuName = (const char *) gles_glGetString(GL_RENDERER);
    return gpuName ? gpuName : "<unknown>";
}

void set_es_version() {
    LOAD_GLES_FUNC(glGetString);
    const char* ESVersion = getBeforeThirdSpace((const char*)gles_glGetString(GL_VERSION));
    int major, minor;

    if (sscanf(ESVersion, "OpenGL ES %d.%d", &major, &minor) == 2) {
        hardware->es_version = major * 100 + minor * 10;
    } else {
        hardware->es_version = 300;
    }
    LOG_I("OpenGL ES Version: %s (%d)", ESVersion, hardware->es_version);
    if (hardware->es_version < 300) {
        LOG_I("OpenGL ES version is lower than 3.0! This version is not supported!");
        LOG_I("Disable glslang and SPIRV-Cross. Using vgpu to process all shaders!");
    }
}

const char* getGLESName() {
    LOAD_GLES_FUNC(glGetString);
    char* ESVersion = (char*)gles_glGetString(GL_VERSION);
    return getBeforeThirdSpace(ESVersion);
}

static std::string rendererString;
static std::string versionString;
const GLubyte * glGetString( GLenum name ) {
    LOG();
    LOAD_GLES_FUNC(glGetString);
    /* Feature in the Future
     * Advanced OpenGL driver: NV renderer.
    switch (name) {
        case GL_VENDOR:
            return (const GLubyte *) "NVIDIA Corporation";
        case GL_VERSION:
            return (const GLubyte *) "4.6.0 NVIDIA 572.16";
        case GL_RENDERER:
            return (const GLubyte *) "NVIDIA GeForce RTX 5090/PCIe/SSE2";
        case GL_SHADING_LANGUAGE_VERSION:
            return (const GLubyte *) "4.50 MobileGlues with glslang and SPIRV-Cross";
        case GL_EXTENSIONS:
            return (const GLubyte *) GetExtensionsList();
    }
    */
    switch (name) {
        case GL_VENDOR:
            return (const GLubyte *)(std::string("Swung0x48, BZLZHH, Tungsten") + 
            std::string(version_type == VERSION_ALPHA ? " | §c§l内测版本, 严禁任何外传!§r" : "")).c_str();
        case GL_VERSION:
            if (versionString == std::string("")) {
                versionString = "4.0.0 MobileGlues";
                std::string realVersion = " " + std::to_string(MAJOR) + "." +
                                      std::to_string(MINOR) + "." +
                                      std::to_string(REVISION);
                std::string suffix = version_type == VERSION_ALPHA ? " | §4§l如果您在公开平台看到这一提示, 则发布者已违规!§r" :
                                     realVersion + std::string(version_type == VERSION_DEVELOPMENT?".Dev":"");
                versionString += suffix;
            }
            return (const GLubyte *)versionString.c_str();
        case GL_RENDERER: 
        {
            if (rendererString == std::string("")) {
                const char* gpuName = getGpuName();
                const char* glesName = getGLESName();
                rendererString = std::string(gpuName) + " | " + std::string(glesName);
            }
            return (const GLubyte *)rendererString.c_str();
        }
        case GL_SHADING_LANGUAGE_VERSION:
            return (const GLubyte *) "4.50 MobileGlues with glslang and SPIRV-Cross";
        case GL_EXTENSIONS:
            return (const GLubyte *) GetExtensionsList();
        default:
            return gles_glGetString(name);
    }
}

const GLubyte * glGetStringi(GLenum name, GLuint index) {
    LOG();
    LOAD_GLES_FUNC(glGetStringi);
    typedef struct {
        GLenum name;
        const char** parts;
        GLuint count;
    } StringCache;
    static StringCache caches[] = {
            {GL_EXTENSIONS, nullptr, 0},
            {GL_VENDOR, nullptr, 0},
            {GL_VERSION, nullptr, 0},
            {GL_SHADING_LANGUAGE_VERSION, nullptr, 0}
    };
    static int initialized = 0;
    if (!initialized) {
        for (auto & cache : caches) {
            GLenum target = cache.name;
            const GLubyte* str = nullptr;
            const char* delimiter = " ";

            switch (target) {
                case GL_VENDOR:
                    str = (const GLubyte*)"Swung0x48, BZLZHH, Tungsten";
                    delimiter = ", ";
                    break;
                case GL_VERSION:
                    str = (const GLubyte*)"4.0.0 MobileGlues";
                    delimiter = " .";
                    break;
                case GL_SHADING_LANGUAGE_VERSION:
                    str = (const GLubyte*)"4.50 MobileGlues with glslang and SPIRV-Cross";
                    break;
                case GL_EXTENSIONS:
                    str = glGetString(GL_EXTENSIONS);
                    break;
                default:
                    return gles_glGetStringi(name, index);
            }

            if (!str) continue;

            char* copy = strdup((const char*)str);
            char* token = strtok(copy, delimiter);
            while (token) {
                cache.parts = (const char**)realloc(cache.parts, (cache.count + 1) * sizeof(char*));
                cache.parts[cache.count++] = strdup(token);
                token = strtok(nullptr, delimiter);
            }
            free(copy);
        }
        initialized = 1;
    }

    for (auto & cache : caches) {
        if (cache.name == name) {
            if (index >= cache.count) {
                return nullptr;
            }
            return (const GLubyte*)cache.parts[index];
        }
    }

    return nullptr;
}

void glGetQueryObjectiv(GLuint id, GLenum pname, GLint* params) {
    LOG()
    LOAD_GLES_FUNC(glGetQueryObjectivEXT)

    gles_glGetQueryObjectivEXT(id, pname, params);
}

void glGetQueryObjecti64v(GLuint id, GLenum pname, GLint64* params) {
    LOG()
    LOAD_GLES_FUNC(glGetQueryObjecti64vEXT)

    gles_glGetQueryObjecti64vEXT(id, pname, params);
}
