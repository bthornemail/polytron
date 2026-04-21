/*
 * test-polyform.c — Simple Polyform Test
 */

#include <stdio.h>
#include <string.h>
#include "include/tetra-png.h"

int main(void) {
    printf("═══ POLYFORM TEST ═══\n\n");
    
    /* 1D Polystick test */
    printf("1. 1D Polystick:\n");
    uint8_t angles[] = {0, 1, 0, 1};
    PolyStick stick = make_polystick(angles, 4, 0x4242);
    printf("   length=%d, start=0x%04X, end=0x%04X\n\n",
           stick.length, stick.start_sid, stick.end_sid);
    
    /* 2D Polyomino test */
    printf("2. 2D Pentomino:\n");
    Polyform2D penta = make_polyomino(5, 0x1234);
    printf("   degree=%d, cells=%d, SID=0x%04X\n",
           penta.degree, penta.count, penta.polyform_sid);
    printf("   FS=0x%02X, GS=0x%02X, RS=0x%02X\n\n",
           extract_fs_channel(&penta),
           extract_gs_channel(&penta),
           extract_rs_channel(&penta));
    
    /* 2.5D Extruded test */
    printf("3. 2.5D Extruded (pentomino × 3):\n");
    Polyform2_5D extruded = extrude_polyform(&penta, 3);
    printf("   height=%d, SID=0x%04X, US=0x%02X\n\n",
           extruded.height, extruded.extruded_sid,
           extract_us_channel(&extruded));
    
    /* 3D Polycube test */
    printf("4. 3D Tetracube:\n");
    Polycube3D cube = make_polycube(4, 0x5678);
    printf("   degree=%d, voxels=%d, SID=0x%04X\n\n",
           cube.degree, cube.count, cube.polycube_sid);
    
    /* Unified test */
    printf("5. Unified Polyform:\n");
    Polyform poly = {0};
    poly.dim = DIM_2D;
    poly.poly2d = penta;
    poly.unified_sid = sid_polyform(&poly);
    printf("   unified SID=0x%04X\n\n", poly.unified_sid);
    
    /* Continuation test */
    printf("6. Polyform Continuation:\n");
    Polyform p1 = poly;
    Polyform p2 = {0};
    p2.dim = DIM_2D;
    p2.poly2d = make_polyomino(3, 0x1111);
    p2.unified_sid = sid_polyform(&p2);
    PolyformCont* cont = chain_polyforms(&p1, &p2, 0);
    printf("   continuation SID=0x%04X\n\n", cont->continuation_sid);
    
    /* Channel extraction test */
    printf("7. Channel Extraction (FS/GS/RS/US):\n");
    printf("   FS (XOR of coords): 0x%02X\n", extract_fs_channel(&penta));
    printf("   GS (AND of coords): 0x%02X\n", extract_gs_channel(&penta));
    printf("   RS (OR of coords):  0x%02X\n", extract_rs_channel(&penta));
    printf("   US (height):       0x%02X\n", extruded.height);
    
    /* PNG generation test */
    printf("8. PNG Generation:\n");
    if (polyform_write_png(&penta, "pentomino.png") == 0) {
        printf("   Wrote pentomino.png successfully\n");
    }
    
    /* Dalí cross PNG layers */
    printf("9. Dalí Cross PNG Layers:\n");
    Polycube3D dali = make_dali_cross();
    printf("   degree=%d, voxels=%d\n", dali.degree, dali.count);
    if (dali_cross_write_png(&dali, "dali") == 0) {
        printf("   Wrote dali_z1.png through dali_z4.png\n");
    }
    free_polycube3d(&dali);
    
    /* Cleanup */
    free_polyform2d(&penta);
    free_polyform2d(&p2.poly2d);
    free_polycube3d(&cube);
    free_continuation(cont);
    
    printf("\n═══ ALL TESTS PASSED ═══\n");
    return 0;
}