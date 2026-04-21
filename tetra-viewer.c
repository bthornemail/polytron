/*
 * TETRA-VIEWER — OpenGL ES Viewer for Polyform PNGs
 *
 * Renders constitutional polyform PNGs using OpenGL ES 2.0.
 * Designed for QEMU RISC-V with VirGL acceleration.
 *
 * COMPILE (RISC-V):
 *   riscv64-linux-gnu-gcc -o tetra-viewer tetra-viewer.c \
 *       -lGLESv2 -lEGL -lpng -lm
 *
 * COMPILE (Native with software GL):
 *   gcc -o tetra-viewer tetra-viewer.c -lGL -lEGL -lpng -lm -DUSE_GL=1
 *
 * QEMU RUN:
 *   qemu-system-riscv64 -device virtio-gpu-gl -display sdl,gl=on ...
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <png.h>

#ifdef USE_GL
#include <GL/gl.h>
#include <EGL/egl.h>
#else
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#endif

/* -------------------------------------------------------------------------- */
/* EMBEDDED KERNEL (from polyform.h)                                         */
/* -------------------------------------------------------------------------- */

typedef uint16_t Pair;

#define cons(a,d) ((((a) & 0xFF) << 8) | ((d) & 0xFF))
#define car(p)    (((p) >> 8) & 0xFF)
#define cdr(p)    ((p) & 0xFF)

#define CONSTITUTIONAL_C  0x1D

static Pair rotl(Pair x, int n) {
    n &= 15;
    return (Pair)((x << n) | (x >> (16 - n)));
}

static Pair rotr(Pair x, int n) {
    n &= 15;
    return (Pair)((x >> n) | (x << (16 - n)));
}

static Pair K(Pair p, Pair C) {
    return rotl(p, 1) ^ rotl(p, 3) ^ rotr(p, 2) ^ C;
}

static Pair compute_sid(Pair nf) {
    return K(nf, CONSTITUTIONAL_C);
}

/* -------------------------------------------------------------------------- */
/* CONSTITUTIONAL PALETTE (16 colors)                                        */
/* -------------------------------------------------------------------------- */

static const uint8_t CONSTITUTIONAL_PALETTE[16 * 3] = {
    0x00, 0x00, 0x00,  /* 0: FS - black */
    0x1D, 0x1D, 0x1D,  /* 1: GS - constitutional gray */
    0xFF, 0x00, 0x00,  /* 2: RS - red */
    0x00, 0xFF, 0x00,  /* 3: US - green */
    0x00, 0x00, 0xFF,  /* 4: p - blue */
    0xFF, 0xFF, 0x00,  /* 5: q - yellow */
    0xFF, 0x00, 0xFF,  /* 6: r - magenta */
    0x00, 0xFF, 0xFF,  /* 7: s - cyan */
    0x80, 0x80, 0x80,  /* 8: gray */
    0xC0, 0x00, 0x00,  /* 9: dark red */
    0x00, 0xC0, 0x00,  /* 10: dark green */
    0x00, 0x00, 0xC0,  /* 11: dark blue */
    0x80, 0x00, 0x00,  /* 12: maroon */
    0x00, 0x80, 0x00,  /* 13: dark green */
    0x00, 0x00, 0x80,  /* 14: navy */
    0x80, 0x80, 0x00   /* 15: olive */
};

/* -------------------------------------------------------------------------- */
/* PNG LOADING (simplified)                                                   */
/* -------------------------------------------------------------------------- */

typedef struct {
    int width;
    int height;
    uint8_t* rgba;
} TetraImage;

static TetraImage* load_png(const char* filename) {
    FILE* fp = fopen(filename, "rb");
    if (!fp) return NULL;
    
    png_byte header[8];
    if (fread(header, 1, 8, fp) != 8) {
        fclose(fp);
        return NULL;
    }
    
    if (png_sig_cmp(header, 0, 8) != 0) {
        fclose(fp);
        return NULL;
    }
    
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info_ptr = png_create_info_struct(png_ptr);
    
    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);
        return NULL;
    }
    
    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8);
    png_read_info(png_ptr, info_ptr);
    
    int width = png_get_image_width(png_ptr, info_ptr);
    int height = png_get_image_height(png_ptr, info_ptr);
    
    TetraImage* img = (TetraImage*)calloc(1, sizeof(TetraImage));
    img->width = width;
    img->height = height;
    img->rgba = (uint8_t*)malloc(width * height * 4);
    
    png_bytep row = (png_bytep)malloc(width);
    for (int y = 0; y < height; y++) {
        png_read_row(png_ptr, row, NULL);
        for (int x = 0; x < width; x++) {
            uint8_t idx = row[x];
            if (idx < 16) {
                img->rgba[(y * width + x) * 4 + 0] = CONSTITUTIONAL_PALETTE[idx * 3 + 0];
                img->rgba[(y * width + x) * 4 + 1] = CONSTITUTIONAL_PALETTE[idx * 3 + 1];
                img->rgba[(y * width + x) * 4 + 2] = CONSTITUTIONAL_PALETTE[idx * 3 + 2];
                img->rgba[(y * width + x) * 4 + 3] = 0xFF;
            } else {
                img->rgba[(y * width + x) * 4 + 0] = 0x1D;
                img->rgba[(y * width + x) * 4 + 1] = 0x1D;
                img->rgba[(y * width + x) * 4 + 2] = 0x1D;
                img->rgba[(y * width + x) * 4 + 3] = 0xFF;
            }
        }
    }
    
    free(row);
    png_read_end(png_ptr, info_ptr);
    png_destroy_read_struct(&png_ptr, &info_ptr);
    fclose(fp);
    
    return img;
}

static void free_image(TetraImage* img) {
    if (!img) return;
    free(img->rgba);
    free(img);
}

/* -------------------------------------------------------------------------- */
/* OPENGL ES RENDERING                                                        */
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

typedef struct {
    void* display;
    void* context;
    void* surface;
    void* config;
    GLuint program;
    GLuint texture;
    int width;
    int height;
} Viewer;

static Viewer* viewer_init(int width, int height) {
    Viewer* v = (Viewer*)calloc(1, sizeof(Viewer));
    v->width = width;
    v->height = height;
    
    /* EGL initialization */
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        free(v);
        return NULL;
    }
    
    EGLint major, minor;
    eglInitialize(display, &major, &minor);
    
    EGLint config_attribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_NONE
    };
    
    EGLConfig config;
    EGLint num_configs;
    eglChooseConfig(display, config_attribs, &config, 1, &num_configs);
    
    EGLContext context = eglCreateContext(display, config, EGL_NO_CONTEXT,
                                          (EGLint[]){EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE});
    
    EGLSurface surface = eglCreateWindowSurface(display, config, 0,
                                                 (EGLint[]){EGL_WIDTH, width, EGL_HEIGHT, height, EGL_NONE});
    
    eglMakeCurrent(display, surface, surface, context);
    
    v->display = (void*)display;
    v->context = (void*)context;
    v->surface = (void*)surface;
    v->config = (void*)config;
    
    return v;
}

static void viewer_load_texture(Viewer* v, TetraImage* img) {
    if (v->texture) glDeleteTextures(1, &v->texture);
    
    glGenTextures(1, &v->texture);
    glBindTexture(GL_TEXTURE_2D, v->texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 img->width, img->height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, img->rgba);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

static void viewer_render(Viewer* v) {
    glViewport(0, 0, v->width, v->height);
    glClearColor(0.07f, 0.07f, 0.07f, 1.0f);  /* #121212 */
    glClear(GL_COLOR_BUFFER_BIT);
    
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, v->texture);
    
    /* Fullscreen quad */
    GLfloat vertices[] = {
        -1.0f, -1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 1.0f,
        -1.0f,  1.0f,  0.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 0.0f,
    };
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 16, vertices);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 16, vertices + 2);
    glEnableVertexAttribArray(1);
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    eglSwapBuffers((EGLDisplay)v->display, (EGLSurface)v->surface);
}

static void viewer_free(Viewer* v) {
    if (!v) return;
    if (v->texture) glDeleteTextures(1, &v->texture);
    eglMakeCurrent((EGLDisplay)v->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroySurface((EGLDisplay)v->display, (EGLSurface)v->surface);
    eglDestroyContext((EGLDisplay)v->display, (EGLContext)v->context);
    eglTerminate((EGLDisplay)v->display);
    free(v);
}

/* -------------------------------------------------------------------------- */
/* MAIN                                                                       */
/* -------------------------------------------------------------------------- */

int main(int argc, char** argv) {
    const char* filename = "pentomino.png";
    int width = 800;
    int height = 600;
    
    if (argc > 1) filename = argv[1];
    if (argc > 3) {
        width = atoi(argv[2]);
        height = atoi(argv[3]);
    }
    
    printf("═══ TETRA-VIEWER ═══\n");
    printf("Loading: %s\n", filename);
    printf("Window: %dx%d\n", width, height);
    
    /* Load PNG */
    TetraImage* img = load_png(filename);
    if (!img) {
        fprintf(stderr, "Failed to load PNG: %s\n", filename);
        return 1;
    }
    printf("Image: %dx%d\n", img->width, img->height);
    
    /* Extract SID from filename (simple hash) */
    Pair sid = 0;
    for (int i = 0; filename[i]; i++) {
        sid = K(sid, (Pair)filename[i]);
    }
    printf("SID: 0x%04X\n", sid);
    
    /* Initialize viewer */
    Viewer* viewer = viewer_init(width, height);
    if (!viewer) {
        fprintf(stderr, "Failed to initialize viewer\n");
        free_image(img);
        return 1;
    }
    
    /* Load texture */
    viewer_load_texture(viewer, img);
    
    /* Render loop (single frame for now) */
    printf("Rendering...\n");
    viewer_render(viewer);
    
    /* Cleanup */
    viewer_free(viewer);
    free_image(img);
    
    printf("═══ DONE ═══\n");
    return 0;
}
