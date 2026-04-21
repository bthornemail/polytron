/*
 * TETRA-GPU — OpenGL ES Utilities for Tetragrammatron
 *
 * Provides GPU texture creation from polyform PNG base objects.
 * Works with VirGL (QEMU), DRI, and native OpenGL ES.
 */

#ifndef TETRA_GPU_H
#define TETRA_GPU_H

#include <stdint.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>

/* -------------------------------------------------------------------------- */
/* GPU CONTEXT                                                                */
/* -------------------------------------------------------------------------- */

typedef struct {
    EGLDisplay egl_display;
    EGLContext egl_context;
    EGLSurface egl_surface;
    EGLConfig egl_config;
    GLuint program;
    GLuint texture;
    int width;
    int height;
} TetraGPU;

/* -------------------------------------------------------------------------- */
/* SHADER SOURCES                                                             */
/* -------------------------------------------------------------------------- */

static const char* VERTEX_SHADER = 
    "attribute vec2 a_position;\n"
    "attribute vec2 a_texcoord;\n"
    "varying vec2 v_texcoord;\n"
    "void main() {\n"
    "    gl_Position = vec4(a_position, 0.0, 1.0);\n"
    "    v_texcoord = a_texcoord;\n"
    "}\n";

static const char* FRAGMENT_SHADER = 
    "#ifdef GL_ES\n"
    "precision mediump float;\n"
    "#endif\n"
    "varying vec2 v_texcoord;\n"
    "uniform sampler2D u_texture;\n"
    "void main() {\n"
    "    gl_FragColor = texture2D(u_texture, v_texcoord);\n"
    "}\n";

/* -------------------------------------------------------------------------- */
/* CONTEXT INITIALIZATION                                                     */
/* -------------------------------------------------------------------------- */

static TetraGPU* tetra_gpu_init(int width, int height) {
    TetraGPU* gpu = (TetraGPU*)calloc(1, sizeof(TetraGPU));
    gpu->width = width;
    gpu->height = height;
    
    /* EGL config for OpenGL ES */
    EGLint config_attribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_NONE
    };
    
    EGLint context_attribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };
    
    EGLint surface_attribs[] = {
        EGL_WIDTH, width,
        EGL_HEIGHT, height,
        EGL_NONE
    };
    
    /* Get EGL display */
    gpu->egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (gpu->egl_display == EGL_NO_DISPLAY) {
        free(gpu);
        return NULL;
    }
    
    /* Initialize EGL */
    EGLint major, minor;
    if (!eglInitialize(gpu->egl_display, &major, &minor)) {
        free(gpu);
        return NULL;
    }
    
    /* Choose config */
    EGLint num_configs;
    if (!eglChooseConfig(gpu->egl_display, config_attribs, 
                        &gpu->egl_config, 1, &num_configs) || 
        num_configs == 0) {
        free(gpu);
        return NULL;
    }
    
    /* Create context */
    gpu->egl_context = eglCreateContext(gpu->egl_display, gpu->egl_config,
                                         EGL_NO_CONTEXT, context_attribs);
    if (gpu->egl_context == EGL_NO_CONTEXT) {
        free(gpu);
        return NULL;
    }
    
    /* Create surface (use native window if available) */
    gpu->egl_surface = eglCreateWindowSurface(gpu->egl_display, gpu->egl_config,
                                              0, surface_attribs);
    if (gpu->egl_surface == EGL_NO_SURFACE) {
        eglDestroyContext(gpu->egl_display, gpu->egl_context);
        free(gpu);
        return NULL;
    }
    
    /* Make current */
    if (!eglMakeCurrent(gpu->egl_display, gpu->egl_surface,
                       gpu->egl_surface, gpu->egl_context)) {
        eglDestroySurface(gpu->egl_display, gpu->egl_surface);
        eglDestroyContext(gpu->egl_display, gpu->egl_context);
        free(gpu);
        return NULL;
    }
    
    return gpu;
}

static void tetra_gpu_free(TetraGPU* gpu) {
    if (!gpu) return;
    
    if (gpu->texture) glDeleteTextures(1, &gpu->texture);
    if (gpu->program) glDeleteProgram(gpu->program);
    
    eglMakeCurrent(gpu->egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE,
                   EGL_NO_CONTEXT);
    eglDestroySurface(gpu->egl_display, gpu->egl_surface);
    eglDestroyContext(gpu->egl_display, gpu->egl_context);
    eglTerminate(gpu->egl_display);
    
    free(gpu);
}

/* -------------------------------------------------------------------------- */
/* SHADER COMPILATION                                                         */
/* -------------------------------------------------------------------------- */

static GLuint tetra_compile_shader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    if (!shader) return 0;
    
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    
    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        GLchar log[512];
        glGetShaderInfoLog(shader, sizeof(log), NULL, log);
        glDeleteShader(shader);
        return 0;
    }
    
    return shader;
}

static GLuint tetra_create_program(const char* vs, const char* fs) {
    GLuint vert = tetra_compile_shader(GL_VERTEX_SHADER, vs);
    GLuint frag = tetra_compile_shader(GL_FRAGMENT_SHADER, fs);
    
    if (!vert || !frag) {
        if (vert) glDeleteShader(vert);
        if (frag) glDeleteShader(frag);
        return 0;
    }
    
    GLuint program = glCreateProgram();
    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);
    
    GLint linked;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) {
        GLchar log[512];
        glGetProgramInfoLog(program, sizeof(log), NULL, log);
        glDeleteProgram(program);
        return 0;
    }
    
    glDeleteShader(vert);
    glDeleteShader(frag);
    
    return program;
}

/* -------------------------------------------------------------------------- */
/* TEXTURE CREATION FROM RGBA DATA                                           */
/* -------------------------------------------------------------------------- */

static GLuint tetra_create_texture(int width, int height, const uint8_t* rgba) {
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, rgba);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    return texture;
}

/* -------------------------------------------------------------------------- */
/* FULLSCREEN QUAD RENDERING                                                 */
/* -------------------------------------------------------------------------- */

static void tetra_render_quad(TetraGPU* gpu) {
    GLfloat vertices[] = {
        /* position    texcoord */
        -1.0f, -1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 1.0f,
        -1.0f,  1.0f,  0.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 0.0f,
    };
    
    GLfloat* v = vertices;
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 16, v);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 16, v + 2);
    glEnableVertexAttribArray(1);
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

/* -------------------------------------------------------------------------- */
/* CONSTITUTIONAL COLOR CONVERSION                                            */
/* -------------------------------------------------------------------------- */

/* Convert 8-color constitutional palette to RGBA */
static void tetra_palette_to_rgba(const uint8_t* palette, uint8_t* rgba) {
    for (int i = 0; i < 16; i++) {
        rgba[i*4 + 0] = palette[i*3 + 0];  /* R */
        rgba[i*4 + 1] = palette[i*3 + 1];  /* G */
        rgba[i*4 + 2] = palette[i*3 + 2];  /* B */
        rgba[i*4 + 3] = 0xFF;              /* A */
    }
}

#endif /* TETRA_GPU_H */
