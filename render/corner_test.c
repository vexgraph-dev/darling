#include "annotation/overview.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "buffer/buffer.h"
#include "buffer/color_buffer.h"
#include "render/raster.h"

;;OVERVIEW
/**
 * ============================================================================
 * MODULE: CornerTest (render/corner_test.c — rounded-rect artifact harness)
 * LEVEL: L3 — Module Code (standalone verification harness)
 * ============================================================================
 * Phase 1 substrate artifact (layout-panels §8): renders the same rounded
 * rect in both corner modes into Buffers and diffs them. Asserts the same
 * bounding box, quadrant symmetry, arc-mode pixel-exactness against the
 * analytic circle on a sampled ring, and superellipse pixels strictly
 * inside the box. Exits 0 on pass, 1 with a printed reason on fail.
 *
 * Pixel convention (shared with Raster_roundedRect): pixel (col, row) is
 * sampled at its center (col+0.5, row+0.5); the top-left arc center sits at
 * (x+radius, y+radius). Integer-exact boundary math uses doubled coords so
 * (2i+1-2r)^2+(2j+1-2r)^2 <= (2r)^2 needs no floating point.
 *
 * STRUCT FIELDS: none — procedural (allocates, renders, checks, frees).
 *
 * FUNCTION REGISTRY:
 * ----------------------------------------------------------------------------
 * Core Functions:
 *   - main(void)
 * ============================================================================
 */

#define TEST_W 64
#define TEST_H 48
#define TEST_X 8
#define TEST_Y 8
#define TEST_WD 48
#define TEST_HT 32
#define TEST_RAD 10

static bool painted(const Buffer *buf, int col, int row) {
    uint8_t r = 0, g = 0, b = 0, a = 0;
    ColorBuffer_getRGBA(buf, (size_t)col, (size_t)row, &r, &g, &b, &a);
    return a != 0;
}

static int fail(const char *reason) {
    printf("FAIL: %s\n", reason);
    return 1;
}

int main(void) {
    Buffer *arc = ColorBuffer(TEST_W, TEST_H);
    Buffer *sup = ColorBuffer(TEST_W, TEST_H);
    Buffer *flat = ColorBuffer(TEST_W, TEST_H);
    Buffer *rect = ColorBuffer(TEST_W, TEST_H);
    if (!arc || !sup || !flat || !rect) {
        printf("FAIL: buffer alloc\n");
        return 1;
    }

    // Radius 0 must equal Raster_rect exactly.
    Raster_roundedRect(flat, TEST_X, TEST_Y, TEST_WD, TEST_HT, 0, 0, 255, 255, 255, 255);
    Raster_rect(rect, TEST_X, TEST_Y, TEST_WD, TEST_HT, 255, 255, 255, 255);
    for (int row = 0; row < TEST_H; row++)
        for (int col = 0; col < TEST_W; col++)
            if (painted(flat, col, row) != painted(rect, col, row)) {
                Buffer_free(arc);
                Buffer_free(sup);
                Buffer_free(flat);
                Buffer_free(rect);
                return fail("radius 0 != Raster_rect");
            }
    Buffer_free(flat);
    Buffer_free(rect);

    Raster_roundedRect(arc, TEST_X, TEST_Y, TEST_WD, TEST_HT, TEST_RAD, 0, 255, 255, 255, 255);
    Raster_roundedRect(sup, TEST_X, TEST_Y, TEST_WD, TEST_HT, TEST_RAD, 1, 255, 255, 255, 255);

    int arcMinX = TEST_W, arcMinY = TEST_H, arcMaxX = -1, arcMaxY = -1;
    int supMinX = TEST_W, supMinY = TEST_H, supMaxX = -1, supMaxY = -1;
    long arcCount = 0;
    long supCount = 0;
    for (int row = 0; row < TEST_H; row++) {
        for (int col = 0; col < TEST_W; col++) {
            bool pa = painted(arc, col, row);
            bool ps = painted(sup, col, row);
            // Strictly inside the box: nothing may paint outside the rect.
            bool inBox = col >= TEST_X && col < TEST_X + TEST_WD
                && row >= TEST_Y && row < TEST_Y + TEST_HT;
            if ((pa || ps) && !inBox) {
                Buffer_free(arc);
                Buffer_free(sup);
                return fail("paint outside box");
            }
            // Superellipse (n=4) contains the circle: arc paint implies super paint.
            if (pa && !ps) {
                Buffer_free(arc);
                Buffer_free(sup);
                return fail("arc pixel missing from superellipse");
            }
            // Quadrant symmetry: exact mirror equality on both axes.
            int mirX = 2 * TEST_X + TEST_WD - 1 - col;
            int mirY = 2 * TEST_Y + TEST_HT - 1 - row;
            if (pa != painted(arc, mirX, row) || pa != painted(arc, col, mirY)) {
                Buffer_free(arc);
                Buffer_free(sup);
                return fail("arc quadrant asymmetry");
            }
            if (ps != painted(sup, mirX, row) || ps != painted(sup, col, mirY)) {
                Buffer_free(arc);
                Buffer_free(sup);
                return fail("superellipse quadrant asymmetry");
            }
            if (pa) {
                arcCount++;
                if (col < arcMinX)
                    arcMinX = col;
                if (col > arcMaxX)
                    arcMaxX = col;
                if (row < arcMinY)
                    arcMinY = row;
                if (row > arcMaxY)
                    arcMaxY = row;
            }
            if (ps) {
                supCount++;
                if (col < supMinX)
                    supMinX = col;
                if (col > supMaxX)
                    supMaxX = col;
                if (row < supMinY)
                    supMinY = row;
                if (row > supMaxY)
                    supMaxY = row;
            }
        }
    }

    // Same bounding box: both modes must span the full rect (edge midpoints
    // always paint) and agree with each other.
    if (arcMinX != TEST_X || arcMinY != TEST_Y
        || arcMaxX != TEST_X + TEST_WD - 1 || arcMaxY != TEST_Y + TEST_HT - 1) {
        Buffer_free(arc);
        Buffer_free(sup);
        return fail("arc bounding box != rect box");
    }
    if (supMinX != arcMinX || supMinY != arcMinY || supMaxX != arcMaxX || supMaxY != arcMaxY) {
        Buffer_free(arc);
        Buffer_free(sup);
        return fail("mode bounding box mismatch");
    }
    // The n=4 curve is strictly fatter than the circle: more pixels painted.
    if (supCount <= arcCount) {
        Buffer_free(arc);
        Buffer_free(sup);
        return fail("superellipse not strictly fatter than arc");
    }

    // Arc pixel-exact vs the analytic circle: for each column of the
    // top-left corner square, the deepest inside pixel paints and the next
    // one out does not. Doubled integer coords: (2i+1-2r)^2+(2j+1-2r)^2<=4r^2.
    long rr = (long)TEST_RAD;
    for (long i = 0; i < rr; i++) {
        long bound = -1;
        for (long j = 0; j < rr; j++) {
            long ax = 2 * i + 1 - 2 * rr;
            long ay = 2 * j + 1 - 2 * rr;
            if (ax * ax + ay * ay <= 4 * rr * rr)
                bound = j;
        }
        if (bound < 0) {
            Buffer_free(arc);
            Buffer_free(sup);
            return fail("analytic ring found no inside pixel");
        }
        if (!painted(arc, TEST_X + (int)i, TEST_Y + (int)bound)) {
            Buffer_free(arc);
            Buffer_free(sup);
            return fail("arc inside-ring pixel unpainted");
        }
        if (bound + 1 < rr && painted(arc, TEST_X + (int)i, TEST_Y + (int)(bound + 1))) {
            Buffer_free(arc);
            Buffer_free(sup);
            return fail("arc outside-ring pixel painted");
        }
    }

    Buffer_free(arc);
    Buffer_free(sup);
    printf("corner_test: ok\n");
    return 0;
}
