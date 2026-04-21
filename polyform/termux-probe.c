/*
 * termux-probe.c - Benchmark probe for Android/Termux
 * 
 * Probes the device capabilities and benchmarks the polyform engine
 * to determine optimal targeting (native vs QEMU user mode)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <unistd.h>

#ifdef __ANDROID__
#include <android/log.h>
#define LOG_TAG "PolyformProbe"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#else
#define LOGI(...) printf(__VA_ARGS__)
#endif

/* Kernel K(p,C) for benchmarking */
static uint32_t rotl32(uint32_t x, int n) {
    n &= 31;
    return (x << n) | (x >> (32 - n));
}

static uint32_t rotr32(uint32_t x, int n) {
    n &= 31;
    return (x >> n) | (x << (32 - n));
}

static uint32_t K(uint32_t p, uint32_t C) {
    return rotl32(p, 1) ^ rotl32(p, 3) ^ rotr32(p, 2) ^ C;
}

/* Benchmark kernel operations */
static double benchmark_kernel(int iterations) {
    clock_t start = clock();
    uint32_t state = 0x4242;
    
    for (int i = 0; i < iterations; i++) {
        state = K(state, 0x1D);
    }
    
    clock_t end = clock();
    return (double)(end - start) / CLOCKS_PER_SEC;
}

/* Benchmark memory operations */
static double benchmark_memory(int iterations) {
    clock_t start = clock();
    
    char *buf = malloc(iterations * 1024);
    if (!buf) return -1;
    
    for (int i = 0; i < iterations * 1024; i++) {
        buf[i % (iterations * 1024)] = (char)i;
    }
    
    free(buf);
    
    clock_t end = clock();
    return (double)(end - start) / CLOCKS_PER_SEC;
}

/* Probe system info */
static void probe_system(void) {
    struct sysinfo info;
    struct utsname uts;
    
    printf("=== System Probe ===\n\n");
    
    /* Kernel info */
    if (uname(&uts) == 0) {
        printf("System:    %s\n", uts.sysname);
        printf("Node:     %s\n", uts.nodename);
        printf("Release:  %s\n", uts.release);
        printf("Machine:  %s\n", uts.machine);
    }
    
    /* Memory info */
    if (sysinfo(&info) == 0) {
        printf("\nMemory:\n");
        printf("  Total:    %lu MB\n", info.totalram / 1024 / 1024);
        printf("  Free:     %lu MB\n", info.freeram / 1024 / 1024);
        printf("  Shared:   %lu MB\n", info.sharedram / 1024 / 1024);
        printf("  Buffers:  %lu MB\n", info.bufferram / 1024 / 1024);
    }
    
    /* CPU info */
    printf("\nCPU:\n");
    printf("  Cores:    %d\n", get_nprocs_conf());
    printf("  Clock:    %ld Hz\n", CLOCKS_PER_SEC);
    
    /* Termux specific */
#ifdef __ANDROID__
    printf("\nPlatform:  Android/Termux\n");
#else
    printf("\nPlatform:  Linux (native)\n");
#endif
    
    /* Check for QEMU */
    FILE *f = fopen("/proc/cpuinfo", "r");
    if (f) {
        char line[256];
        while (fgets(line, sizeof(line), f)) {
            if (strstr(line, "QEMU") || strstr(line, "KVM")) {
                printf("  Emulator: Detected\n");
                break;
            }
        }
        fclose(f);
    }
}

/* Run benchmarks */
static void run_benchmarks(void) {
    int iters[] = {1000, 10000, 100000, 1000000};
    int n = sizeof(iters) / sizeof(iters[0]);
    
    printf("\n=== Benchmarks ===\n\n");
    
    printf("Kernel K(p,C) Operations:\n");
    printf("%-12s %12s %12s\n", "Iterations", "Time (s)", "Ops/sec");
    printf("%-12s %12s %12s\n", "----------", "--------", "-------");
    
    for (int i = 0; i < n; i++) {
        double elapsed = benchmark_kernel(iters[i]);
        printf("%-12d %12.6f %12.0f\n", iters[i], elapsed, 
               elapsed > 0 ? iters[i] / elapsed : 0);
    }
    
    printf("\nMemory Operations (1KB writes):\n");
    printf("%-12s %12s %12s\n", "Iterations", "Time (s)", "MB/sec");
    printf("%-12s %12s %12s\n", "----------", "--------", "-------");
    
    for (int i = 0; i < n; i++) {
        double elapsed = benchmark_memory(iters[i]);
        printf("%-12d %12.6f %12.2f\n", iters[i], elapsed,
               elapsed > 0 ? (iters[i] * 1.0) / elapsed : 0);
    }
    
    /* Recommendation */
    printf("\n=== Recommendation ===\n");
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        long total_mb = info.totalram / 1024 / 1024;
        if (total_mb < 512) {
            printf("Low memory (%ld MB). Use native build, not QEMU.\n", total_mb);
        } else if (total_mb < 2048) {
            printf("Medium memory (%ld MB). Native or QEMU user mode OK.\n", total_mb);
        } else {
            printf("High memory (%ld MB). Full QEMU system possible.\n", total_mb);
        }
    }
}

/* Test polyform generation */
static void test_polyforms(void) {
    printf("\n=== Polyform Generation Test ===\n\n");
    
    /* Quick test of K-map */
    int kmap_iters = 10000;
    clock_t start = clock();
    
    uint32_t state = 0x4242;
    int true_count = 0;
    
    for (int i = 0; i < kmap_iters; i++) {
        state = K(state, 0x1D);
        if (state & 1) true_count++;
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("K-map (4-var):    %d iters in %.3fs = %.0f/sec\n",
           kmap_iters, elapsed, kmap_iters / elapsed);
    printf("True ratio:       %.1f%%\n", 100.0 * true_count / kmap_iters);
    
    /* SVG write test */
    printf("\nSVG write test:\n");
    FILE *f = fopen("polyform/test-termux.svg", "w");
    if (f) {
        fprintf(f, "<?xml version=\"1.0\"?>\n");
        fprintf(f, "<svg xmlns=\"http://www.w3.org/2000/svg\" ");
        fprintf(f, "viewBox=\"0 0 100 100\">\n");
        fprintf(f, "  <rect width=\"100\" height=\"100\" fill=\"white\"/>\n");
        fprintf(f, "  <text x=\"10\" y=\"50\">Polyform Test - Termux</text>\n");
        fprintf(f, "</svg>\n");
        fclose(f);
        printf("  Wrote: polyform/test-termux.svg\n");
    } else {
        printf("  ERROR: Cannot write to polyform/\n");
    }
}

int main(int argc, char *argv[]) {
    printf("========================================\n");
    printf("   Polyform Logic Engine - Termux Probe\n");
    printf("========================================\n\n");
    
    /* Always show system info */
    probe_system();
    
    /* Check for benchmark flag */
    int do_bench = 1;
    int do_test = 1;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--sys-only") == 0) {
            do_bench = 0;
            do_test = 0;
        } else if (strcmp(argv[i], "--bench-only") == 0) {
            do_test = 0;
        } else if (strcmp(argv[i], "--test-only") == 0) {
            do_bench = 0;
        }
    }
    
    if (do_bench) {
        run_benchmarks();
    }
    
    if (do_test) {
        test_polyforms();
    }
    
    printf("\n========================================\n");
    printf("   Probe Complete\n");
    printf("========================================\n");
    
    return 0;
}
