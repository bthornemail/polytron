/*
 * TETRA-HOST-VIEWER — Host-side IVSHMEM viewer
 * 
 * mmaps /dev/shm/mesh_fb and renders it as an OpenGL ES texture.
 * Zero-copy: framebuffer writes appear immediately.
 * 
 * COMPILE:
 *   gcc -o tetra-host-viewer tetra-host-viewer.c -lGLESv2 -lEGL -lrt -lm -lpthread
 * 
 * RUN:
 *   # First create shared memory (as root or with permissions)
 *   truncate -s 16M /dev/shm/mesh_fb
 *   chmod 666 /dev/shm/mesh_fb
 *   ./tetra-host-viewer
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>

/* -------------------------------------------------------------------------- */
/* CONFIG                                                                      */
/* -------------------------------------------------------------------------- */

#define FB_WIDTH      512
#define FB_HEIGHT     512
#define FB_SIZE       (FB_WIDTH * FB_HEIGHT * 4)
#define SHM_PATH      "/mesh_fb"
#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

/* -------------------------------------------------------------------------- */
/* EMBEDDED KERNEL                                                            */
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
/* CONSTITUTIONAL PALETTE (BGRA for little-endian)                           */
/* -------------------------------------------------------------------------- */

static const uint32_t CONSTITUTIONAL_PALETTE[16] = {
    0xFF000000,  /* 0: FS - black */
    0xFF1D1D1D,  /* 1: GS - constitutional gray */
    0xFF0000FF,  /* 2: RS - red */
    0xFF00FF00,  /* 3: US - green */
    0xFFFF0000,  /* 4: p - blue */
    0xFF00FFFF,  /* 5: q - yellow */
    0xFFFF00FF,  /* 6: r - magenta */
    0xFFFFFF00,  /* 7: s - cyan */
    0xFF808080,  /* 8: gray */
    0xFF0000C0,  /* 9: dark red */
    0xFF00C000,  /* 10: dark green */
    0xFFC00000,  /* 11: dark blue */
    0xFF000080,  /* 12: maroon */
    0xFF008000,  /* 13: dark green */
    0xFF800000,  /* 14: navy */
    0xFF008080   /* 15: olive */
};

/* -------------------------------------------------------------------------- */
/* GLOBAL STATE                                                               */
/* -------------------------------------------------------------------------- */

static uint32_t* raw_fb = NULL;
static GLuint texture = 0;
static EGLDisplay display = EGL_NO_DISPLAY;
static EGLSurface surface = EGL_NO_SURFACE;
static EGLContext context = EGL_NO_CONTEXT;
static int running = 1;

/* -------------------------------------------------------------------------- */
/* EGL / GLES INITIALIZATION                                                 */
/* -------------------------------------------------------------------------- */

static int init_gles(int width, int height) {
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        fprintf(stderr, "Failed to get EGL display\n");
        return -1;
    }
    
    EGLint major, minor;
    if (!eglInitialize(display, &major, &minor)) {
        fprintf(stderr, "Failed to initialize EGL\n");
        return -1;
    }
    printf("EGL version: %d.%d\n", major, minor);
    
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
    if (!eglChooseConfig(display, config_attribs, &config, 1, &num_configs) || num_configs == 0) {
        fprintf(stderr, "Failed to choose EGL config\n");
        return -1;
    }
    
    EGLint context_attribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };
    
    context = eglCreateContext(display, config, EGL_NO_CONTEXT, context_attribs);
    if (context == EGL_NO_CONTEXT) {
        fprintf(stderr, "Failed to create EGL context\n");
        return -1;
    }
    
    EGLint surface_attribs[] = {
        EGL_WIDTH, width,
        EGL_HEIGHT, height,
        EGL_NONE
    };
    
    surface = eglCreateWindowSurface(display, config, 0, surface_attribs);
    if (surface == EGL_NO_SURFACE) {
        fprintf(stderr, "Failed to create EGL surface\n");
        return -1;
    }
    
    if (!eglMakeCurrent(display, surface, surface, context)) {
        fprintf(stderr, "Failed to make EGL context current\n");
        return -1;
    }
    
    /* Create texture */
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glEnable(GL_TEXTURE_2D);
    
    printf("OpenGL ES 2.0 initialized\n");
    return 0;
}

static void cleanup_gles(void) {
    if (texture) glDeleteTextures(1, &texture);
    if (display != EGL_NO_DISPLAY) {
        eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (surface != EGL_NO_SURFACE) eglDestroySurface(display, surface);
        if (context != EGL_NO_CONTEXT) eglDestroyContext(display, context);
        eglTerminate(display);
    }
}

/* -------------------------------------------------------------------------- */
/* SHADERS                                                                   */
/* -------------------------------------------------------------------------- */

static GLuint create_program(void) {
    const char* vs_source = 
        "attribute vec2 a_position;\n"
        "attribute vec2 a_texcoord;\n"
        "varying vec2 v_texcoord;\n"
        "void main() {\n"
        "    gl_Position = vec4(a_position, 0.0, 1.0);\n"
        "    v_texcoord = a_texcoord;\n"
        "}\n";
    
    const char* fs_source = 
        "#ifdef GL_ES\n"
        "precision mediump float;\n"
        "#endif\n"
        "varying vec2 v_texcoord;\n"
        "uniform sampler2D u_texture;\n"
        "void main() {\n"
        "    gl_FragColor = texture2D(u_texture, v_texcoord);\n"
        "}\n";
    
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vs_source, NULL);
    glCompileShader(vs);
    
    GLint compiled;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        char log[512];
        glGetShaderInfoLog(vs, sizeof(log), NULL, log);
        fprintf(stderr, "VS error: %s\n", log);
        return 0;
    }
    
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fs_source, NULL);
    glCompileShader(fs);
    
    glGetShaderiv(fs, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        char log[512];
        glGetShaderInfoLog(fs, sizeof(log), NULL, log);
        fprintf(stderr, "FS error: %s\n", log);
        return 0;
    }
    
    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    
    GLint linked;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) {
        char log[512];
        glGetProgramInfoLog(program, sizeof(log), NULL, log);
        fprintf(stderr, "Link error: %s\n", log);
        return 0;
    }
    
    glUseProgram(program);
    
    return program;
}

/* -------------------------------------------------------------------------- */
/* RENDER                                                                     */
/* -------------------------------------------------------------------------- */

static void render_frame(void) {
    if (!raw_fb) return;
    
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glClearColor(0.07f, 0.07f, 0.07f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    /* Update texture - zero copy from shared memory */
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, FB_WIDTH, FB_HEIGHT, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, raw_fb);
    
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
    
    eglSwapBuffers(display, surface);
}

/* -------------------------------------------------------------------------- */
/* IVSHMEM MAPPING                                                           */
/* -------------------------------------------------------------------------- */

static int init_shmem(void) {
    int fd = shm_open(SHM_PATH, O_RDWR, 0666);
    if (fd < 0) {
        perror("shm_open");
        
        /* Try to create it */
        fd = shm_open(SHM_PATH, O_CREAT | O_RDWR, 0666);
        if (fd < 0) {
            perror("shm_open (create)");
            return -1;
        }
        if (ftruncate(fd, FB_SIZE) < 0) {
            perror("ftruncate");
            close(fd);
            return -1;
        }
    }
    
    raw_fb = mmap(NULL, FB_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (raw_fb == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return -1;
    }
    
    close(fd);
    
    printf("IVSHMEM framebuffer mapped at %p, size %d bytes\n", raw_fb, FB_SIZE);
    return 0;
}

static void clear_fb(void) {
    if (raw_fb) {
        memset(raw_fb, 0, FB_SIZE);
    }
}

/* -------------------------------------------------------------------------- */
/* MAIN                                                                      */
/* -------------------------------------------------------------------------- */

int main(int argc, char** argv) {
    printf("═══ TETRA-HOST-VIEWER ═══\n");
    printf("IVSHMEM Zero-Copy Framebuffer Viewer\n");
    printf("\n");
    
    /* Map shared memory */
    if (init_shmem() < 0) {
        fprintf(stderr, "Failed to initialize shared memory\n");
        fprintf(stderr, "Run as root or create: truncate -s 16M /dev/shm/mesh_fb\n");
        return 1;
    }
    
    /* Clear framebuffer */
    clear_fb();
    
    /* Initialize OpenGL ES */
    if (init_gles(WINDOW_WIDTH, WINDOW_HEIGHT) < 0) {
        fprintf(stderr, "Failed to initialize GLES\n");
        return 1;
    }
    
    /* Create shader program */
    GLuint program = create_program();
    if (!program) {
        fprintf(stderr, "Failed to create shader program\n");
        return 1;
    }
    
    printf("\n");
    printf("Framebuffer: %dx%d (RGBA)\n", FB_WIDTH, FB_HEIGHT);
    printf("Window: %dx%d\n", WINDOW_WIDTH, WINDOW_HEIGHT);
    printf("\n");
    printf("Tetragrammatron IVSHMEM Viewer running...\n");
    printf("Press Ctrl+C to exit\n");
    
    /* Render loop */
    int frame = 0;
    while (running) {
        render_frame();
        
        /* Every 60 frames, print stats */
        frame++;
        if (frame % 60 == 0) {
            /* Check for activity in framebuffer */
            uint32_t first_pixel = raw_fb[0];
            printf("Frame %d: fb[0] = 0x%08X\n", frame, first_pixel);
        }
        
        usleep(16666);  /* ~60 FPS */
    }
    
    printf("\n═══ SHUTDOWN ═══\n");
    
    cleanup_gles();
    
    if (raw_fb) {
        munmap(raw_fb, FB_SIZE);
    }
    
    return 0;
}
