cmake_minimum_required(VERSION 3.22.1)

project("mobileglues")

enable_language(CXX)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w -fvisibility=hidden -funwind-tables -g -D_THREAD_SAFE -fPIC -stdlib=libc++")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w -g -std=gnu99 -funwind-tables -O3 -fvisibility=hidden")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++")

set(CMAKE_ANDROID_STL_TYPE c++_static)

set(CMAKE_BUILD_TYPE Release)

find_library(GLSLANG_LIB glslang PATHS ${CMAKE_SOURCE_DIR}/libraries/arm64-v8a/)

add_library(${CMAKE_PROJECT_NAME} SHARED
        init.cpp
        main.c
        gl/gl_stub.c
        gl/gl_native.c
        gl/gl.c
        gl/envvars.c
        gl/log.c
        gl/program.c
        gl/shader.c
        gl/framebuffer.c
        gl/texture.cpp
        gl/drawing.c
        gl/mg.c
        gl/buffer.cpp
        gl/getter.cpp
        gl/glsl/glsl_for_es.cpp
        glx/lookup.c
        egl/egl.c
        egl/loader.c
        gles/loader.c
        config/cJSON.c
        config/config.c
        config/gpu_utils.c
        config/nsbypass.c
        config/settings.c
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ./includes)

target_link_libraries(${CMAKE_PROJECT_NAME}
        ${CMAKE_SOURCE_DIR}/libraries/arm64-v8a/libglslang.a
        ${CMAKE_SOURCE_DIR}/libraries/arm64-v8a/libspirv-cross-c-shared.so
        ${CMAKE_SOURCE_DIR}/libraries/arm64-v8a/libshaderconv.so
        android
        log
        EGL
        vulkan
)

target_include_directories(mobileglues PUBLIC
        ${CMAKE_SOURCE_DIR}/include
        )