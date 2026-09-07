#include "annotation/overview.h"

#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#include "darling/container.h"
#include "darling/field/slider.h"
#include "event/pointer.h"
#include "nio/mem.h"

;;OVERVIEW
/**
 * ============================================================================
 * MODULE: SliderTest (darling/field/slider_test.c)
 * LEVEL: L3 — Module Code (headless verification harness)
 * ============================================================================
 * Headless suite for Slider pointer drag, range clamping, step snapping,
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
    if (cond) printf("[slider_test] PASS %s\n", name); \
    else { printf("[slider_test] FAIL %s\n", name); g_failures++; } \
} while (0)

static bool nearf(float a, float b, float eps) {
    return fabsf(a - b) <= eps;
}

static void onSliderChange(void *ctx) {
    bool *fired = (bool*) ctx;
    if (fired) (*fired) = true;
}

int main(void) {
    printf("=== Running Slider Test Suite ===\n");

    // §1 Null safety
    Slider_handlePointer(nullptr, PTR_DOWN, 50.0f, 10.0f);
    Slider_setRange(nullptr, 0.0f, 100.0f);
    CHECK("null safety", true);

    // §2 Horizontal slider drag and callback
    Slider *s = Slider_0();
    CHECK("Slider_0 created", s != nullptr);

    Panel *p = &(*s).base;
    Container *cnt = &(*p).base;
    (*cnt).w = 200.0f;
    (*cnt).h = 20.0f;

    Slider_setRange(s, 0.0f, 100.0f);
    Slider_setStep(s, 0.0f);
    CHECK("range initialized", nearf(Slider_getMin(s), 0.0f, 0.001f) && nearf(Slider_getMax(s), 100.0f, 0.001f));

    bool fired = false;
    Slider_setOnChange(s, onSliderChange);
    Slider_setCtx(s, &fired);

    // Pointer DOWN at midpoint (100px of 200px => 50.0)
    Slider_handlePointer(s, PTR_DOWN, 100.0f, 10.0f);
    CHECK("pointer down updates value to 50", nearf(Slider_getValue(s), 50.0f, 0.001f));
    CHECK("pointer down fires callback", fired == true);

    // Pointer DRAG to 150px (75% => 75.0)
    fired = false;
    Slider_handlePointer(s, PTR_DRAG, 150.0f, 10.0f);
    CHECK("pointer drag updates value to 75", nearf(Slider_getValue(s), 75.0f, 0.001f));
    CHECK("pointer drag fires callback", fired == true);

    // Pointer MOVE does not change value or fire callback
    fired = false;
    Slider_handlePointer(s, PTR_MOVE, 50.0f, 10.0f);
    CHECK("pointer move ignored", nearf(Slider_getValue(s), 75.0f, 0.001f));
    CHECK("pointer move does not fire callback", fired == false);

    // Same value drag does not re-fire callback
    fired = false;
    Slider_handlePointer(s, PTR_DRAG, 150.0f, 10.0f);
    CHECK("same position drag does not fire callback", fired == false);

    // §3 Clamping outside bounds
    Slider_handlePointer(s, PTR_DRAG, -50.0f, 10.0f);
    CHECK("clamp below min", nearf(Slider_getValue(s), 0.0f, 0.001f));

    Slider_handlePointer(s, PTR_DRAG, 300.0f, 10.0f);
    CHECK("clamp above max", nearf(Slider_getValue(s), 100.0f, 0.001f));

    // §4 Step snapping
    Slider_setStep(s, 10.0f);
    // 47px on 200px => ratio 0.235 => val 23.5 => snapped to 20.0
    Slider_handlePointer(s, PTR_DRAG, 47.0f, 10.0f);
    CHECK("step snap rounds to nearest step", nearf(Slider_getValue(s), 20.0f, 0.001f));

    // 56px on 200px => ratio 0.28 => val 28.0 => snapped to 30.0
    Slider_handlePointer(s, PTR_DRAG, 56.0f, 10.0f);
    CHECK("step snap rounds up to nearest step", nearf(Slider_getValue(s), 30.0f, 0.001f));

    // §5 Vertical slider
    Slider_setVertical(s, true);
    Slider_setStep(s, 0.0f);
    fired = false;
    // localY = 5.0f on h = 20.0f => ratio = 0.25 => val = 25.0
    Slider_handlePointer(s, PTR_DRAG, 10.0f, 5.0f);
    CHECK("vertical slider tracks localY", nearf(Slider_getValue(s), 25.0f, 0.001f));
    CHECK("vertical drag fires callback", fired == true);

    // §6 Slider_setRange clamps existing value
    Slider_setValue(s, 95.0f);
    Slider_setRange(s, 10.0f, 80.0f);
    CHECK("setRange clamps high value", nearf(Slider_getValue(s), 80.0f, 0.001f));
    Slider_setValue(s, 5.0f);
    Slider_setRange(s, 20.0f, 80.0f);
    CHECK("setRange clamps low value", nearf(Slider_getValue(s), 20.0f, 0.001f));

    Memory_free(s);

    printf("\n=== Slider Test Summary: %d failures ===\n", g_failures);
    return g_failures > 0 ? 1 : 0;
}
