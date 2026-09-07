#include "annotation/overview.h"

#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#include "darling/container.h"
#include "darling/field/scrollbar.h"
#include "event/pointer.h"
#include "nio/mem.h"

;;OVERVIEW
/**
 * ============================================================================
 * MODULE: ScrollBarTest (darling/field/scrollbar_test.c)
 * LEVEL: L3 — Module Code (headless verification harness)
 * ============================================================================
 * Headless suite for ScrollBar pointer drag, clickAt fraction mapping,
 * range clamping, and mode handling. Exits 0 on pass, 1 on failure.
 *
 * STRUCT FIELDS: none — procedural test harness.
 *
 * FUNCTION REGISTRY:
 * ----------------------------------------------------------------------------
 * Core Functions:
 *   - main(void)
 * ============================================================================
 */

static int g_failures = 0;

#define CHECK(name, cond) do { \
    if (cond) printf("[scrollbar_test] PASS %s\n", name); \
    else { printf("[scrollbar_test] FAIL %s\n", name); g_failures++; } \
} while (0)

static bool nearf(float a, float b, float eps) {
    return fabsf(a - b) <= eps;
}

int main(void) {
    printf("=== Running ScrollBar Test Suite ===\n");

    // §1 Null safety
    ScrollBar_handlePointer(nullptr, PTR_DOWN, 10.0f, 50.0f);
    ScrollBar_setRange(nullptr, 0.0f, 100.0f);
    CHECK("null safety", true);

    // §2 ScrollBar creation and setup
    ScrollBar *s = ScrollBar_0();
    CHECK("ScrollBar_0 created", s != nullptr);

    Panel *p = &(*s).base;
    Container *cnt = &(*p).base;
    (*cnt).w = 20.0f;
    (*cnt).h = 200.0f;

    ScrollBar_setRange(s, 0.0f, 1000.0f);
    CHECK("range set to [0, 1000]", nearf(ScrollBar_getValue(s), 0.0f, 0.001f));

    // §3 Pointer DOWN at midpoint (localY = 100 on h = 200 => fraction 0.5 => val = 500)
    ScrollBar_handlePointer(s, PTR_DOWN, 10.0f, 100.0f);
    CHECK("pointer down maps midpoint to 500", nearf(ScrollBar_getValue(s), 500.0f, 0.01f));

    // §4 Pointer DRAG to 150px (localY = 150 on h = 200 => fraction 0.75 => val = 750)
    ScrollBar_handlePointer(s, PTR_DRAG, 10.0f, 150.0f);
    CHECK("pointer drag maps 75% to 750", nearf(ScrollBar_getValue(s), 750.0f, 0.01f));

    // §5 Pointer MOVE does not change value
    ScrollBar_handlePointer(s, PTR_MOVE, 10.0f, 50.0f);
    CHECK("pointer move ignored", nearf(ScrollBar_getValue(s), 750.0f, 0.01f));

    // §6 Pointer beyond bounds clamped to min/max
    ScrollBar_handlePointer(s, PTR_DRAG, 10.0f, -50.0f);
    CHECK("pointer drag clamps to min 0", nearf(ScrollBar_getValue(s), 0.0f, 0.01f));

    ScrollBar_handlePointer(s, PTR_DRAG, 10.0f, 250.0f);
    CHECK("pointer drag clamps to max 1000", nearf(ScrollBar_getValue(s), 1000.0f, 0.01f));

    // §7 Default height fallback (h <= 0 defaults to 100.0)
    (*cnt).h = 0.0f;
    ScrollBar_handlePointer(s, PTR_DOWN, 10.0f, 30.0f);
    // fraction = 30 / 100 = 0.3 => val = 300
    CHECK("default height fallback maps 30px to 300", nearf(ScrollBar_getValue(s), 300.0f, 0.01f));

    Memory_free(s);

    printf("\n=== ScrollBar Test Summary: %d failures ===\n", g_failures);
    return g_failures > 0 ? 1 : 0;
}
