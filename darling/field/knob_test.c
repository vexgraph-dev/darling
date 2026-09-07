#include "annotation/overview.h"

#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#include "darling/container.h"
#include "darling/field/knob.h"
#include "event/pointer.h"
#include "nio/mem.h"

;;OVERVIEW
/**
 * ============================================================================
 * MODULE: KnobTest (darling/field/knob_test.c)
 * LEVEL: L3 — Module Code (headless verification harness)
 * ============================================================================
 * Headless suite for Knob angular drag, normalized mapping, bounds clamping,
 * and change callback delivery. Exits 0 on pass, 1 on failure.
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
    if (cond) printf("[knob_test] PASS %s\n", name); \
    else { printf("[knob_test] FAIL %s\n", name); g_failures++; } \
} while (0)

static bool nearf(float a, float b, float eps) {
    return fabsf(a - b) <= eps;
}

static void onKnobChange(void *ctx) {
    bool *fired = (bool*) ctx;
    if (fired) (*fired) = true;
}

int main(void) {
    printf("=== Running Knob Test Suite ===\n");

    // §1 Null safety
    Knob_handlePointer(nullptr, PTR_DOWN, 20.0f, 20.0f);
    Knob_setNormalized(nullptr, 0.5f);
    CHECK("null safety", true);

    // §2 Knob creation and setup
    Knob *k = Knob_0();
    CHECK("Knob_0 created", k != nullptr);

    Panel *p = &(*k).base;
    Container *cnt = &(*p).base;
    (*cnt).w = 40.0f;
    (*cnt).h = 40.0f;
    Knob_setDiameter(k, 40.0f);
    Knob_setMin(k, 0.0f);
    Knob_setMax(k, 100.0f);

    bool fired = false;
    Knob_setOnChange(k, onKnobChange);
    Knob_setCtx(k, &fired);

    // §3 Normalized setter
    Knob_setNormalized(k, 0.5f);
    CHECK("setNormalized(0.5) updates value to 50", nearf(Knob_getValue(k), 50.0f, 0.001f));
    CHECK("setNormalized fires callback", fired == true);

    // Same value setter does not fire callback
    fired = false;
    Knob_setNormalized(k, 0.5f);
    CHECK("same normalized does not fire callback", fired == false);

    // §4 Clamping normalized
    Knob_setNormalized(k, -0.2f);
    CHECK("setNormalized clamps below zero", nearf(Knob_getValue(k), 0.0f, 0.001f));

    Knob_setNormalized(k, 1.5f);
    CHECK("setNormalized clamps above one", nearf(Knob_getValue(k), 100.0f, 0.001f));

    // §5 Pointer interaction (rotary mapping)
    // Center is (cx=20, cy=20)
    // Local (40, 20) => dx=20, dy=0 => angle=0 => norm = (0+PI)/(2PI) = 0.5 => val = 50.0
    fired = false;
    Knob_handlePointer(k, PTR_DOWN, 40.0f, 20.0f);
    CHECK("pointer down at 3 o'clock maps to 50%", nearf(Knob_getValue(k), 50.0f, 0.01f));
    CHECK("pointer down fires callback", fired == true);

    // Local (20, 40) => dx=0, dy=20 => angle=PI/2 => norm = (PI/2+PI)/(2PI) = 0.75 => val = 75.0
    fired = false;
    Knob_handlePointer(k, PTR_DRAG, 20.0f, 40.0f);
    CHECK("pointer drag to 6 o'clock maps to 75%", nearf(Knob_getValue(k), 75.0f, 0.01f));
    CHECK("pointer drag fires callback", fired == true);

    // Local (20, 0) => dx=0, dy=-20 => angle=-PI/2 => norm = (-PI/2+PI)/(2PI) = 0.25 => val = 25.0
    fired = false;
    Knob_handlePointer(k, PTR_DRAG, 20.0f, 0.0f);
    CHECK("pointer drag to 12 o'clock maps to 25%", nearf(Knob_getValue(k), 25.0f, 0.01f));
    CHECK("pointer drag fires callback", fired == true);

    // Non-DOWN/DRAG pointer event ignored
    fired = false;
    Knob_handlePointer(k, PTR_MOVE, 40.0f, 20.0f);
    CHECK("pointer move ignored", nearf(Knob_getValue(k), 25.0f, 0.01f));
    CHECK("pointer move does not fire callback", fired == false);

    Memory_free(k);

    printf("\n=== Knob Test Summary: %d failures ===\n", g_failures);
    return g_failures > 0 ? 1 : 0;
}
