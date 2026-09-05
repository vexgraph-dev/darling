#include "event/bridge.h"

#include "event/dispatch.h"
#include "event/keyevent.h"
#include "event/pointer.h"
#include "darling/panel/panel.h"
#include "event/keyhandler.h"
#include "event/mousehandler.h"
#include "input/key.h"
#include "input/mouse.h"
#include "nio/mem.h"
#include "time/nanotime.h"
#include "annotation/overview.h"

;;OVERVIEW
/**
 * ============================================================================
 * MODULE: EventBridge (procedural handoff, no struct, no type id)
 * LEVEL: L2 — Behavior (raw input translation into UI events)
 * ============================================================================
 * The third part of the input path: vexspoke freezes capture-time
 * timestamps at push (slot.pressTime + packed epoch micros); this module
 * translates each raw listener callback into a darling UI event and
 * fires it synchronously. Down/up carry exactNanos end to end, so the
 * press moment is what registers. Move/drag are stamped at delivery
 * (documented: motion is never judgment-critical).
 *
 * FOCUS MODEL: explicit only. The app sets the key target with
 * Darling_bridgeSetFocused; auto-focus on click lands later.
 *
 * DEFERRED: char composition, scroll/zoom/delta, touch gestures.
 *
 * STRUCT FIELDS: none — procedural handoff (operates on Panel tree +
 * event structs). File statics below hold the binding, same pattern as
 * hotcwap loaders: s_root (pointer tree), s_focused (key target),
 * s_attached (listener guard), s_keyListener/s_mouseListener (vtables).
 *
 * FUNCTION REGISTRY:
 * ----------------------------------------------------------------------------
 * Core Functions:
 *   - Darling_bridgeAttach(root)
 *   - Darling_bridgeDetach()
 *
 * Setters:
 *   - Darling_bridgeSetFocused(p)
 *
 * Getters:
 *   - Darling_bridgeGetFocused()
 *   - Darling_bridgeGetRoot()
 * ============================================================================
 */

static Panel *s_root = nullptr;
static Panel *s_focused = nullptr;
static bool s_attached = false;
static KeyHandler s_keyListener;
static MouseHandler s_mouseListener;

// --- raw key callbacks (exactNanos = capture-frozen, passthrough) -----------

static void bridgeKeyDown(void *self, int keyEvent, uint64_t exactNanos) {
    (void)self;
    Panel *f = s_focused;
    if (!f)
        return;
    UIKeyEvent *ev = UIKeyEvent_0();
    if (!ev)
        return;
    uint32_t mods = 0u;
    if (keyEvent & KEY_MOD_SHIFT)
        mods |= 1u;
    if (keyEvent & KEY_MOD_CONTROL)
        mods |= 2u;
    if (keyEvent & KEY_MOD_OPTION)
        mods |= 4u;
    if (keyEvent & KEY_MOD_COMMAND)
        mods |= 8u;
    UIKeyEvent_setTarget(ev, f);
    UIKeyEvent_setKeyCode(ev, keyEvent & KEY_MASK_CODE);
    UIKeyEvent_setCh(ev, -1);
    UIKeyEvent_setMods(ev, mods);
    UIKeyEvent_setPressed(ev, true);
    UIKeyEvent_setRepeat(ev, false);
    UIKeyEvent_setNanos(ev, exactNanos);
    Darling_fireKey(f, ev);
    Memory_free(ev);
}

static void bridgeKeyUp(void *self, int keyEvent, uint64_t exactNanos) {
    (void)self;
    Panel *f = s_focused;
    if (!f)
        return;
    UIKeyEvent *ev = UIKeyEvent_0();
    if (!ev)
        return;
    uint32_t mods = 0u;
    if (keyEvent & KEY_MOD_SHIFT)
        mods |= 1u;
    if (keyEvent & KEY_MOD_CONTROL)
        mods |= 2u;
    if (keyEvent & KEY_MOD_OPTION)
        mods |= 4u;
    if (keyEvent & KEY_MOD_COMMAND)
        mods |= 8u;
    UIKeyEvent_setTarget(ev, f);
    UIKeyEvent_setKeyCode(ev, keyEvent & KEY_MASK_CODE);
    UIKeyEvent_setCh(ev, -1);
    UIKeyEvent_setMods(ev, mods);
    UIKeyEvent_setPressed(ev, false);
    UIKeyEvent_setRepeat(ev, false);
    UIKeyEvent_setNanos(ev, exactNanos);
    Darling_fireKey(f, ev);
    Memory_free(ev);
}

static void bridgeKeyRepeat(void *self, int keyEvent, uint64_t exactNanos) {
    (void)self;
    Panel *f = s_focused;
    if (!f)
        return;
    UIKeyEvent *ev = UIKeyEvent_0();
    if (!ev)
        return;
    UIKeyEvent_setTarget(ev, f);
    UIKeyEvent_setKeyCode(ev, keyEvent & KEY_MASK_CODE);
    UIKeyEvent_setCh(ev, -1);
    UIKeyEvent_setPressed(ev, true);
    UIKeyEvent_setRepeat(ev, true);
    UIKeyEvent_setNanos(ev, exactNanos);
    Darling_fireKey(f, ev);
    Memory_free(ev);
}

// --- raw mouse callbacks -----------------------------------------------------

static void bridgeMouseDown(void *self, int mouseEvent, uint64_t exactNanos) {
    (void)self;
    Panel *root = s_root;
    if (!root)
        return;
    int button = Mouse_button(mouseEvent);
    double x = Mouse_x();
    double y = Mouse_y();
    PointerEvent *ev = PointerEvent_4(PTR_DOWN, (float)x, (float)y, button);
    if (!ev)
        return;
    PointerEvent_setNanos(ev, exactNanos);
    Darling_firePointer(root, ev);
    Memory_free(ev);
}

static void bridgeMouseUp(void *self, int mouseEvent, uint64_t exactNanos) {
    (void)self;
    Panel *root = s_root;
    if (!root)
        return;
    int button = Mouse_button(mouseEvent);
    double x = Mouse_x();
    double y = Mouse_y();
    PointerEvent *ev = PointerEvent_4(PTR_UP, (float)x, (float)y, button);
    if (!ev)
        return;
    PointerEvent_setNanos(ev, exactNanos);
    Darling_firePointer(root, ev);
    Memory_free(ev);
}

static void bridgeMouseMove(void *self, double x, double y) {
    (void)self;
    Panel *root = s_root;
    if (!root)
        return;
    PointerEvent *ev = PointerEvent_4(PTR_MOVE, (float)x, (float)y, 0);
    if (!ev)
        return;
    PointerEvent_setNanos(ev, NanoTime_now());
    Darling_firePointer(root, ev);
    Memory_free(ev);
}

static void bridgeMouseDrag(void *self, int button, double x, double y) {
    (void)self;
    Panel *root = s_root;
    if (!root)
        return;
    PointerEvent *ev = PointerEvent_4(PTR_DRAG, (float)x, (float)y, button);
    if (!ev)
        return;
    PointerEvent_setNanos(ev, NanoTime_now());
    Darling_firePointer(root, ev);
    Memory_free(ev);
}

// CORE FUNCTIONS
// ============================================================================

void Darling_bridgeAttach(Panel *root) {
    s_root = root;
    if (s_attached)
        return;
    s_keyListener.self = nullptr;
    s_keyListener.onKeyDown = bridgeKeyDown;
    s_keyListener.onKeyUp = bridgeKeyUp;
    s_keyListener.onKeyRepeat = bridgeKeyRepeat;
    s_keyListener.onCharTyped = nullptr;
    s_mouseListener.self = nullptr;
    s_mouseListener.onMouseDown = bridgeMouseDown;
    s_mouseListener.onMouseUp = bridgeMouseUp;
    s_mouseListener.onMouseRepeat = nullptr;
    s_mouseListener.onMouseMove = bridgeMouseMove;
    s_mouseListener.onMouseMoveDelta = nullptr;
    s_mouseListener.onMouseDrag = bridgeMouseDrag;
    s_mouseListener.onMouseScroll = nullptr;
    s_mouseListener.onMouseZoom = nullptr;
    Key_addListener(&s_keyListener);
    Mouse_addListener(&s_mouseListener);
    s_attached = true;
}

void Darling_bridgeDetach(void) {
    if (!s_attached)
        return;
    (void)Key_removeListener(&s_keyListener);
    (void)Mouse_removeListener(&s_mouseListener);
    s_attached = false;
    s_root = nullptr;
    s_focused = nullptr;
}

// SETTERS
// ============================================================================

void Darling_bridgeSetFocused(Panel *p) {
    s_focused = p;
}

// GETTERS
// ============================================================================

Panel *Darling_bridgeGetFocused(void) {
    return s_focused;
}

Panel *Darling_bridgeGetRoot(void) {
    return s_root;
}
