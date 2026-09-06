#include "annotation/overview.h"

#include <math.h>
#include <stdio.h>

#include "darling/anim/anim.h"
#include "darling/button/button.h"
#include "darling/container.h"
#include "darling/field/input.h"
#include "darling/field/slider.h"
#include "darling/label/label.h"
#include "darling/panel/panel.h"
#include "nio/mem.h"

;;OVERVIEW
/**
 * ============================================================================
 * MODULE: AnimTest (darling/anim/anim_test.c — easing + player suite)
 * LEVEL: L3 — Module Code (headless verification harness)
 * ============================================================================
 * Headless suite for the Anim timeline: easing endpoint/symmetry laws,
 * per-section sampling (location/size/scale/font/alpha), loop, cancel,
 * replay-restart, onDone, and per-class facade smoke. Exits 0 on pass,
 * 1 with printed reasons on failure. No window, no GPU.
 *
 * STRUCT FIELDS: none — procedural (allocates, ticks, checks, frees).
 *
 * FUNCTION REGISTRY:
 * ----------------------------------------------------------------------------
 * Core Functions:
 *   - main(void)
 * ============================================================================
 */

static int g_failures = 0;

#define CHECK(name, cond) do { \
    if (cond) { printf("[anim] PASS %s\n", name); } \
    else { printf("[anim] FAIL %s\n", name); g_failures++; } \
} while (0)

static bool nearf(float a, float b, float eps) {
    return fabsf(a - b) <= eps;
}

static int g_doneCount = 0;
static void onDoneCounter(void *ctx) {
    (void)ctx;
    g_doneCount++;
}

int main(void) {
    // §1 Easing endpoint law: every token maps 0->0 and 1->1.
    AnimEase all[] = {
        ANIM_NORMAL,
        ANIM_EASE_IN, ANIM_EASE_OUT, ANIM_EASE_IN_OUT,
        ANIM_SINE_IN, ANIM_SINE_OUT, ANIM_SINE_IN_OUT,
        ANIM_EXPONENTIAL_IN, ANIM_EXPONENTIAL_OUT, ANIM_EXPONENTIAL_IN_OUT,
        ANIM_ELASTIC_IN, ANIM_ELASTIC_OUT, ANIM_ELASTIC_IN_OUT,
        ANIM_BOUNCE_IN, ANIM_BOUNCE_OUT, ANIM_BOUNCE_IN_OUT,
    };
    bool endsOk = true;
    for (size_t i = 0; i < sizeof(all) / sizeof(all[0]); i++) {
        if (Anim_eval(all[i], 0.0f) != 0.0f || Anim_eval(all[i], 1.0f) != 1.0f)
            endsOk = false;
    }
    CHECK("ease endpoints 0->0 1->1 (all 16)", endsOk);

    // §2 Symmetry law: IN_OUT midpoints sit at 0.5 (merged hill).
    CHECK("ease inout midpoint", nearf(Anim_eval(ANIM_EASE_IN_OUT, 0.5f), 0.5f, 1e-5f));
    CHECK("sine inout midpoint", nearf(Anim_eval(ANIM_SINE_IN_OUT, 0.5f), 0.5f, 1e-5f));
    CHECK("expo inout midpoint", nearf(Anim_eval(ANIM_EXPONENTIAL_IN_OUT, 0.5f), 0.5f, 1e-5f));
    CHECK("bounce inout midpoint", nearf(Anim_eval(ANIM_BOUNCE_IN_OUT, 0.5f), 0.5f, 1e-3f));
    CHECK("normal midpoint", nearf(Anim_eval(ANIM_NORMAL, 0.5f), 0.5f, 1e-6f));

    // §3 Expo steeper-than-ease law (pow(x,2) twice): EXPO_IN sits below EASE_IN.
    CHECK("expo steeper than ease",
        Anim_eval(ANIM_EXPONENTIAL_IN, 0.5f) < Anim_eval(ANIM_EASE_IN, 0.5f)
            && nearf(Anim_eval(ANIM_EXPONENTIAL_IN, 0.5f), 0.0625f, 1e-5f)
            && nearf(Anim_eval(ANIM_EASE_IN, 0.5f), 0.25f, 1e-5f));

    // §4 Location section: linear mid + snap at end.
    Container *c = Container_0();
    Container_setLocation(c, 10.0f, 20.0f);
    Anim *mv = Anim_0();
    Anim_addLocation(mv, 1.0f, 110.0f, 70.0f, ANIM_NORMAL);
    CHECK("duration max-t", nearf(Anim_duration(mv), 1.0f, 1e-6f));
    CHECK("keycount location", Anim_keyCount(mv, ANIM_SECTION_LOCATION) == 1);
    Container_animate(c, mv);
    CHECK("playing after play", Anim_isPlaying(c) && Anim_liveCount() == 1);
    Anim_tick(0.5);
    CHECK("location mid", nearf(Container_getX(c), 60.0f, 1e-4f) && nearf(Container_getY(c), 45.0f, 1e-4f));
    Anim_tick(0.5);
    CHECK("location end snap", nearf(Container_getX(c), 110.0f, 1e-4f) && nearf(Container_getY(c), 70.0f, 1e-4f));
    CHECK("stopped at end", !Anim_isPlaying(c) && Anim_liveCount() == 0);

    // §5 First key interpolates FROM the captured start (preset reuse).
    Container_setLocation(c, 0.0f, 0.0f);
    Container_animate(c, mv); // same preset, new start
    Anim_tick(0.25);
    CHECK("preset reuses start", nearf(Container_getX(c), 27.5f, 1e-4f));
    Anim_cancel(c);
    CHECK("cancel stops", !Anim_isPlaying(c));

    // §6 Size + scale sections.
    Anim *sz = Anim_0();
    Container_setSize(c, 100.0f, 50.0f);
    Container_setScale(c, 1.0f, 1.0f);
    Anim_addSize(sz, 2.0f, 200.0f, 100.0f, ANIM_NORMAL);
    Anim_addScale(sz, 2.0f, 2.0f, 3.0f, ANIM_NORMAL);
    Container_animate(c, sz);
    Anim_tick(1.0);
    CHECK("size mid", nearf(Container_getWidth(c), 150.0f, 1e-4f) && nearf(Container_getHeight(c), 75.0f, 1e-4f));
    CHECK("scale mid",
        nearf(Container_getScaleWidth(c), 1.5f, 1e-4f) && nearf(Container_getScaleHeight(c), 2.0f, 1e-4f));
    Anim_cancel(c);

    // §7 Font-size section on Label.
    Label *l = Label_1("hi");
    Label_setFontSize(l, 10.0f);
    Anim *fs = Anim_0();
    Anim_addFontSize(fs, 1.0f, 20.0f, ANIM_NORMAL);
    Label_animate(l, fs);
    Anim_tick(1.0);
    Container *lbase = &((*l).base.base);
    CHECK("label font end", nearf(Label_getFontSize(l), 20.0f, 1e-4f) && !Anim_isPlaying(lbase));

    // §8 Alpha section over the Panel bg channel (0xAARRGGBB: high byte).
    Panel *p = Panel_0();
    Panel_setBackgroundColor(p, 0xFF112233u);
    Anim *al = Anim_0();
    Anim_addAlpha(al, 1.0f, 0.0f, ANIM_NORMAL);
    Panel_animate(p, al);
    Anim_tick(0.5);
    CHECK("alpha mid", (Panel_getBackgroundColor(p) >> 24) == 128u);
    Anim_tick(0.5);
    CHECK("alpha end", (Panel_getBackgroundColor(p) >> 24) == 0u
        && (Panel_getBackgroundColor(p) & 0x00FFFFFFu) == 0x00112233u);

    // §9 Loop wraps instead of finishing.
    Anim *lp = Anim_0();
    Anim_addLocation(lp, 1.0f, 100.0f, 0.0f, ANIM_NORMAL);
    Anim_setLoop(lp, true);
    CHECK("loop getter", Anim_isLoop(lp));
    Container_setLocation(c, 0.0f, 0.0f);
    Container_animate(c, lp);
    Anim_tick(1.5);
    CHECK("loop wraps", Anim_isPlaying(c) && nearf(Container_getX(c), 50.0f, 1e-4f));
    Anim_cancel(c);

    // §10 onDone fires once on natural finish (not on cancel).
    Anim *dn = Anim_0();
    Anim_addLocation(dn, 0.5f, 5.0f, 5.0f, ANIM_NORMAL);
    g_doneCount = 0;
    Anim_setOnDone(dn, onDoneCounter, nullptr);
    CHECK("done getter", Anim_getOnDone(dn) == onDoneCounter);
    Container_animate(c, dn);
    Anim_tick(0.5);
    CHECK("done fired", g_doneCount == 1 && !Anim_isPlaying(c));
    Container_animate(c, dn);
    Anim_cancel(c);
    CHECK("cancel skips done", g_doneCount == 1);

    // §11 Multi-key timeline with mixed easings.
    Anim *mk = Anim_0();
    Anim_addLocation(mk, 1.0f, 100.0f, 0.0f, ANIM_EASE_OUT);
    Anim_addLocation(mk, 2.0f, 100.0f, 100.0f, ANIM_EASE_IN);
    CHECK("multi duration", nearf(Anim_duration(mk), 2.0f, 1e-6f));
    Container_setLocation(c, 0.0f, 0.0f);
    Container_animate(c, mk);
    Anim_tick(2.0);
    CHECK("multi end", nearf(Container_getX(c), 100.0f, 1e-3f) && nearf(Container_getY(c), 100.0f, 1e-3f));

    // §12 Per-class facade smoke (compile + run).
    Button *b = Button_1("go");
    Anim *sm = Anim_0();
    Anim_addScale(sm, 0.5f, 1.2f, 1.2f, ANIM_EASE_OUT);
    Button_animate(b, sm);
    Slider *sl = Slider_0();
    Slider_animate(sl, sm);
    Input *inp = Input_0();
    Input_animate(inp, sm);
    CHECK("facade live x3", Anim_liveCount() == 3);
    Anim_tick(0.5);
    CHECK("facade done x3", Anim_liveCount() == 0);
    Anim_cancelAll();

    Anim_free(mv);
    Anim_free(sz);
    Anim_free(fs);
    Anim_free(al);
    Anim_free(lp);
    Anim_free(dn);
    Anim_free(mk);
    Anim_free(sm);
    Memory_free(c);
    Memory_free(p);
    Label_free(l);
    Button_free(b);
    // Slider/Input have no public free in the shell phase; arena owns them.

    printf("[anim] %s (%d failures)\n", g_failures == 0 ? "ALL PASS" : "FAILURES", g_failures);
    return g_failures != 0;
}
