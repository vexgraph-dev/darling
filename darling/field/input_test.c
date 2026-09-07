#include "annotation/overview.h"

#include <stdio.h>
#include <string.h>

#include "darling/field/input.h"
#include "event/keyevent.h"
#include "event/pointer.h"
#include "input/key.h"
#include "nio/mem.h"

;;OVERVIEW
/**
 * ============================================================================
 * MODULE: InputTest (darling/field/input_test.c)
 * LEVEL: L3 — Module Code (headless verification harness)
 * ============================================================================
 * Headless suite for Input live typing (Pkg 4): insert/erase fire
 * onChange, arrows clamp via goTo, enter fires onSubmit, readonly blocks
 * edits, pointer DOWN requests focus + places the caret, null-safe.
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
    if (cond) { printf("[input_test] PASS %s\n", name); } \
    else { printf("[input_test] FAIL %s\n", name); g_failures++; } \
} while (0)

static int s_changes = 0;
static int s_submits = 0;

static void onChange(void *ctx) {
    s_changes++;
    if (ctx) (*(int*) ctx)++;
}

static void onSubmit(void *ctx) {
    s_submits++;
    (void)ctx;
}

static float fakeMeasure(void *ctx, int32_t index) {
    (void)ctx;
    return (float)index * 10.0f;
}

static UIKeyEvent *makeKey(int32_t code, int32_t ch, bool pressed) {
    UIKeyEvent *ev = UIKeyEvent_0();
    if (ev) {
        UIKeyEvent_setKeyCode(ev, code);
        UIKeyEvent_setCh(ev, ch);
        UIKeyEvent_setPressed(ev, pressed);
    }
    return ev;
}

int main(void) {
    printf("=== Running Input Test Suite ===\n");

    // §1 Null-safe no-ops (reaching here is the pass).
    {
        Input_insertChar(nullptr, 'a');
        Input_eraseChar(nullptr);
        Input_handlePointer(nullptr, PTR_DOWN, 0.0f, 0.0f);
        Input_handleKey(nullptr, nullptr);
        Input *inp = Input_0();
        Input_handleKey(inp, nullptr);
        Input_handleKey(nullptr, makeKey(KEY_A, 'a', true));
        CHECK("null-safe no crash", inp != nullptr);
        Input_free(inp);
        Input_free(nullptr);
    }

    // §2 Insert fires onChange + text correct.
    {
        s_changes = 0;
        int ctxCount = 0;
        Input *inp = Input_0();
        Input_setOnChange(inp, onChange);
        Input_setCtx(inp, &ctxCount);
        Input_insertChar(inp, 'H');
        Input_insertChar(inp, 'i');
        Input_insertChar(inp, '!');
        const char *t = Input_getText(inp);
        CHECK("insert text correct", t != nullptr && strcmp(t, "Hi!") == 0);
        CHECK("insert cursor at end", Input_getCursor(inp) == 3);
        CHECK("insert fires onChange", s_changes == 3 && ctxCount == 3);
        // Mid-buffer insert lands at the cursor.
        Input_goTo(inp, 1);
        Input_insertChar(inp, 'a');
        t = Input_getText(inp);
        CHECK("mid insert text", t != nullptr && strcmp(t, "Hai!") == 0);
        CHECK("mid insert cursor", Input_getCursor(inp) == 2);
        CHECK("mid insert fires", s_changes == 4);
        Input_free(inp);
    }

    // §3 Backspace erases + fires.
    {
        s_changes = 0;
        Input *inp = Input_0();
        Input_setOnChange(inp, onChange);
        Input_setText(inp, "abc");
        Input_goTo(inp, 3);
        Input_eraseChar(inp);
        const char *t = Input_getText(inp);
        CHECK("backspace erases", t != nullptr && strcmp(t, "ab") == 0);
        CHECK("backspace cursor", Input_getCursor(inp) == 2);
        CHECK("backspace fires", s_changes == 1);
        // Backspace at 0: no change, no fire.
        Input_goTo(inp, 0);
        Input_eraseChar(inp);
        t = Input_getText(inp);
        CHECK("backspace at 0 keeps text", t != nullptr && strcmp(t, "ab") == 0);
        CHECK("backspace at 0 silent", s_changes == 1);
        Input_free(inp);
    }

    // §4 Keys: printable insert, backspace, arrows clamp, enter submits.
    {
        s_changes = 0;
        s_submits = 0;
        Input *inp = Input_0();
        Input_setOnChange(inp, onChange);
        Input_setOnSubmit(inp, onSubmit);
        UIKeyEvent *ev = makeKey(KEY_A, 'x', true);
        Input_handleKey(inp, ev);
        Memory_free(ev);
        const char *t = Input_getText(inp);
        CHECK("key printable inserts", t != nullptr && strcmp(t, "x") == 0 && s_changes == 1);
        // Release is ignored.
        ev = makeKey(KEY_A, 'y', false);
        Input_handleKey(inp, ev);
        Memory_free(ev);
        t = Input_getText(inp);
        CHECK("key release ignored", t != nullptr && strcmp(t, "x") == 0 && s_changes == 1);
        // Left clamps at 0, right clamps at len.
        ev = makeKey(KEY_LEFT, -1, true);
        Input_handleKey(inp, ev);
        Input_handleKey(inp, ev);
        Memory_free(ev);
        CHECK("key left clamps", Input_getCursor(inp) == 0);
        ev = makeKey(KEY_RIGHT, -1, true);
        Input_handleKey(inp, ev);
        Input_handleKey(inp, ev);
        Memory_free(ev);
        CHECK("key right clamps", Input_getCursor(inp) == 1);
        // Backspace via keyCode erases.
        ev = makeKey(KEY_BACKSPACE, -1, true);
        Input_handleKey(inp, ev);
        Memory_free(ev);
        t = Input_getText(inp);
        CHECK("key backspace erases", t != nullptr && strcmp(t, "") == 0);
        // Enter submits; release does not.
        ev = makeKey(KEY_ENTER, -1, true);
        Input_handleKey(inp, ev);
        Memory_free(ev);
        CHECK("key enter submits", s_submits == 1);
        ev = makeKey(KEY_ENTER, -1, false);
        Input_handleKey(inp, ev);
        Memory_free(ev);
        CHECK("key enter release silent", s_submits == 1);
        Input_free(inp);
    }

    // §5 Readonly blocks edits, caret still moves.
    {
        s_changes = 0;
        s_submits = 0;
        Input *inp = Input_0();
        Input_setOnChange(inp, onChange);
        Input_setOnSubmit(inp, onSubmit);
        Input_setText(inp, "ro");
        Input_setReadonly(inp, true);
        Input_insertChar(inp, '!');
        const char *t = Input_getText(inp);
        CHECK("readonly blocks insert", t != nullptr && strcmp(t, "ro") == 0 && s_changes == 0);
        Input_eraseChar(inp);
        t = Input_getText(inp);
        CHECK("readonly blocks erase", t != nullptr && strcmp(t, "ro") == 0 && s_changes == 0);
        UIKeyEvent *ev = makeKey(KEY_A, 'q', true);
        Input_handleKey(inp, ev);
        Memory_free(ev);
        t = Input_getText(inp);
        CHECK("readonly blocks key insert", t != nullptr && strcmp(t, "ro") == 0 && s_changes == 0);
        ev = makeKey(KEY_BACKSPACE, -1, true);
        Input_handleKey(inp, ev);
        Memory_free(ev);
        CHECK("readonly blocks key erase", s_changes == 0);
        ev = makeKey(KEY_LEFT, -1, true);
        Input_handleKey(inp, ev);
        Memory_free(ev);
        CHECK("readonly caret at 0 clamps", Input_getCursor(inp) == 0);
        ev = makeKey(KEY_RIGHT, -1, true);
        Input_handleKey(inp, ev);
        Memory_free(ev);
        CHECK("readonly caret still moves", Input_getCursor(inp) == 1);
        Input_setReadonly(inp, false);
        Input_insertChar(inp, '!');
        t = Input_getText(inp);
        CHECK("writable resumes", t != nullptr && strcmp(t, "r!o") == 0 && s_changes == 1);
        Input_free(inp);
    }

    // §6 Pointer DOWN: focus request + caret-to-click.
    {
        Input *inp = Input_0();
        Input_setText(inp, "hello");
        CHECK("starts unfocused", Input_isFocused(inp) == false);
        Input_setMeasurer(inp, fakeMeasure, nullptr);
        Input_handlePointer(inp, PTR_DOWN, 24.0f, 5.0f);
        CHECK("pointer down focuses", Input_isFocused(inp) == true);
        CHECK("pointer caret to click", Input_getCursor(inp) == 2);
        // Other kinds are minimal no-ops.
        Input_handlePointer(inp, PTR_MOVE, 0.0f, 0.0f);
        Input_handlePointer(inp, PTR_UP, 0.0f, 0.0f);
        CHECK("pointer move/up ignored", Input_getCursor(inp) == 2);
        Input_free(inp);
        // No measurer: DOWN goes to the end.
        Input *plain = Input_0();
        Input_setText(plain, "abc");
        Input_handlePointer(plain, PTR_DOWN, 0.0f, 0.0f);
        CHECK("pointer no measurer goes to end", Input_getCursor(plain) == 3);
        CHECK("pointer no measurer focuses", Input_isFocused(plain) == true);
        Input_free(plain);
    }

    // §7 Cap truncates, password changes no logic.
    {
        s_changes = 0;
        Input *inp = Input_2(nullptr, 3);
        Input_setOnChange(inp, onChange);
        Input_insertChar(inp, 'a');
        Input_insertChar(inp, 'b');
        Input_insertChar(inp, 'c');
        Input_insertChar(inp, 'd');
        const char *t = Input_getText(inp);
        CHECK("cap truncates", t != nullptr && strcmp(t, "abc") == 0);
        CHECK("cap drop silent", s_changes == 3);
        Input_setPassword(inp, true);
        Input_goTo(inp, 3);
        Input_eraseChar(inp);
        t = Input_getText(inp);
        CHECK("password keeps logic", t != nullptr && strcmp(t, "ab") == 0 && s_changes == 4);
        Input_setPassword(inp, false);
        Input_free(inp);
    }

    printf("\n=== Input Test Summary: %d failures ===\n", g_failures);
    return g_failures > 0 ? 1 : 0;
}
