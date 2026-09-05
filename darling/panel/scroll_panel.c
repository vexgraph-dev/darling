#include "darling/panel/scroll_panel.h"

#include "darling/field/scrollbar.h"
#include "darling/panel/panel.h"
#include "annotation/overview.h"
#include "nio/mem.h"
#include "oop/type.h"

#include <math.h>

;;OVERVIEW
/**
 * ============================================================================
 * CLASS: ScrollPanel (embeds Panel)
 * LEVEL: L2 — Behavior (clipped viewport behavior API)
 * ============================================================================
 * Viewport over an oversized content panel with start/end offsets, inset
 * padding on both scroll ends, child clipping, an owned vertical ScrollBar,
 * touchscreen feel (fling momentum + slippery friction + overscroll
 * rubber-band), and a content-panel forwarding part. Offsets clamp to
 * [-startInset, content-view+endInset] per axis, extended by the overscroll
 * allowance when rubber-banding is enabled.
 * The offset pair is the single source of truth: the bar writes the offset
 * via syncFromBar, setOffset writes back to the bar via syncToBar, and
 * tick integrates fling velocity into the offset every frame.
 * Content attach is detach-only and never frees.
 *
 * THE SCROLLBAR (what it is, what it does):
 * ----------------------------------------------------------------------------
 * The scrollbar is the owned ScrollBar node (*bar): a vertical track+thumb
 * that MIRRORS offsetY. It does two jobs and nothing else:
 *   1. Display: syncToBar maps the current offsetY into the bar's [min,max]
 *      value so the thumb position always shows where you are.
 *   2. Input:  syncFromBar maps a dragged/clicked bar value back into
 *      offsetY so grabbing the thumb scrolls the content.
 * The bar is a child of the viewport (painted on top, clipped with it).
 * Hide it with ScrollPanel_scrollbar_setVisible when it gets in the way —
 * touch readers, fullscreen galleries, game logs, auto-hiding overlays —
 * and the offsets keep working exactly the same with no thumb on screen.
 * The bar never owns the offset; hiding never disables scrolling.
 *
 * FEEL (touchscreen physics):
 * ----------------------------------------------------------------------------
 *   fling(vx, vy)  — toss the content with an initial velocity (px/sec),
 *                    e.g. from a swipe-release gesture.
 *   slippery 0..1 — how long the glide lasts. 0 stops dead (legacy direct
 *                    manipulation), 1 glides far. Maps to exponential
 *                    friction inside tick; honor-system default 0 so old
 *                    call sites behave bit-identically.
 *   overscroll px — rubber-band allowance past each clamp end. 0 disables
 *                    (hard clamp, legacy). Positive lets drags/flings pop
 *                    past the edge with extra damping, then tick springs
 *                    them home. That iOS "pulled too far and it snaps back"
 *                    is overscroll > 0 plus tick running every frame.
 *   tick(dt)       — advance on Thread 0 (next to layout): integrate
 *                    velocity, decay by friction, spring back overshoot.
 *                    Call it once per frame while isScrolling, or always —
 *                    it is a cheap no-op at rest inside the bounds.
 *
 * CONTENT-PANEL PART (modify the panel through here):
 * ----------------------------------------------------------------------------
 * A C purist would write content->field and pierce the struct. Here the
 * ScrollPanel owns the content relationship, so edits go through the
 * ScrollPanel_panel_* forwarders: panel_setSize, panel_setBackgroundColor,
 * panel_setRadius (+symmetric getters). They no-op on empty viewports and
 * re-clamp the offset after resizes so you never strand the view past the
 * new content end. Anything finer (margins, anchors) stays on Panel itself.
 *
 * STRUCT FIELDS (Mirroring darling/panel/scroll_panel.h — same part banners):
 * ----------------------------------------------------------------------------
 *   --- ScrollPanel core (owner fields) ---
 *   Panel base;                  // Inherited layout, bounds, hierarchy state
 *   Panel *content;              // Viewport child (via panel_*); null = empty
 *   float offsetX;               // Horizontal scroll offset into content
 *   float offsetY;               // Vertical scroll offset into content
 *   float startInset;            // Padding before the first child
 *   float endInset;              // Overscroll past the last child
 *   --- Scrollbar part (live view) ---
 *   ScrollBar *bar;              // Owned vertical bar (replaceable, never freed)
 *   bool barVisible;             // Scrollbar-part visibility (default true)
 *   --- Feel part (owner fields, tick advances) ---
 *   float velX, velY;            // Fling velocity px/sec (tick decays)
 *   float slippery;              // 0 stops dead, 1 long glide (default 0)
 *   float overscroll;            // Rubber-band px past ends (0 = disabled)
 *
 * FUNCTION REGISTRY:
 * ----------------------------------------------------------------------------
 * Constructors:
 *   - ScrollPanel_2(viewW, viewH)
 *
 * Core Functions:
 *   - ScrollPanel_setContent(sp, content)
 *   - ScrollPanel_setOffset(sp, x, y)
 *   - ScrollPanel_syncFromBar(sp)
 *   - ScrollPanel_syncToBar(sp)
 *
 * Setters:
 *   - ScrollPanel_setStartInset(sp, inset)
 *   - ScrollPanel_setEndInset(sp, inset)
 *
 * Scrollbar part:
 *   - ScrollPanel_scrollbar_setVisible(sp, visible)
 *   - ScrollPanel_scrollbar_setBar(sp, bar)   // replace the view, keep sync
 *
 * Feel part:
 *   - ScrollPanel_setSlippery(sp, slippery)
 *   - ScrollPanel_setOverscroll(sp, px)
 *   - ScrollPanel_fling(sp, vx, vy)
 *   - ScrollPanel_stop(sp)
 *   - ScrollPanel_tick(sp, dt)
 *
 * Content-panel part:
 *   - ScrollPanel_panel_setSize(sp, w, h)
 *   - ScrollPanel_panel_setBackgroundColor(sp, color)
 *   - ScrollPanel_panel_setRadius(sp, radius)
 *
 * Getters:
 *   - ScrollPanel_getContent(sp)
 *   - ScrollPanel_getOffset(sp, outX, outY)
 *   - ScrollPanel_getStartInset(sp)
 *   - ScrollPanel_getEndInset(sp)
 *   - ScrollPanel_getBar(sp)
 *   - ScrollPanel_scrollbar_isVisible(sp)
 *   - ScrollPanel_getSlippery(sp)
 *   - ScrollPanel_getOverscroll(sp)
 *   - ScrollPanel_getVelocity(sp, outVX, outVY)
 *   - ScrollPanel_isScrolling(sp)
 *   - ScrollPanel_isOverscrolled(sp)
 *   - ScrollPanel_panel_getSize(sp, outW, outH)
 *   - ScrollPanel_panel_getBackgroundColor(sp)
 *   - ScrollPanel_panel_getRadius(sp)
 * ============================================================================
 */

// CONSTRUCTORS

ScrollPanel *ScrollPanel_2(float viewW, float viewH) {
    ScrollPanel *sp = (ScrollPanel*) Memory_alloc(TYPE_SCROLL_PANEL_SINGLETON, sizeof(ScrollPanel));
    if (!sp)
        return nullptr;
    Panel *b = Panel_0();
    if (!b) {
        Memory_free(sp);
        return nullptr;
    }
    (*sp).base = (*b);
    Memory_free(b);
    (*sp).content = nullptr;
    (*sp).bar = nullptr;
    (*sp).offsetX = 0.0f;
    (*sp).offsetY = 0.0f;
    (*sp).startInset = 0.0f;
    (*sp).endInset = 0.0f;
    (*sp).barVisible = true;
    (*sp).velX = 0.0f;
    (*sp).velY = 0.0f;
    (*sp).slippery = 0.0f;
    (*sp).overscroll = 0.0f;
    Panel *self = &(*sp).base;
    Container *c = &(*self).base;
    Container_setSize(c, viewW, viewH);
    Container_setClipChildren(c, true);
    ScrollBar *bar = ScrollBar_0();
    if (!bar) {
        Memory_free(sp);
        return nullptr;
    }
    Panel *thumb = &(*bar).base;
    Panel_addContainer(self, thumb);
    (*sp).bar = bar;
    return sp;
}

// CORE FUNCTIONS

static float pinOffset(float value, float lo, float hi) {
    if (value < lo)
        return lo;
    if (value > hi)
        return hi;
    return value;
}

static void markDirty(ScrollPanel *sp) {
    if (!sp)
        return;
    Panel *b = &(*sp).base;
    Container_markDirty(&(*b).base);
}

static void offsetBounds(const ScrollPanel *sp, float *loX, float *hiX, float *loY, float *hiY) {
    float start = (*sp).startInset;
    float end = (*sp).endInset;
    const Panel *b = &(*sp).base;
    const Container *vc = &(*b).base;
    float viewW = Container_getWidth(vc);
    float viewH = Container_getHeight(vc);
    float contentW = 0.0f;
    float contentH = 0.0f;
    Panel *content = (*sp).content;
    if (content) {
        Container *cc = &(*content).base;
        contentW = Container_getWidth(cc);
        contentH = Container_getHeight(cc);
    }
    float lx = -start;
    float hx = contentW - viewW + end;
    float ly = -start;
    float hy = contentH - viewH + end;
    if (hx < lx)
        hx = lx;
    if (hy < ly)
        hy = ly;
    if (loX)
        (*loX) = lx;
    if (hiX)
        (*hiX) = hx;
    if (loY)
        (*loY) = ly;
    if (hiY)
        (*hiY) = hy;
}

void ScrollPanel_setContent(ScrollPanel *sp, Panel *content) {
    if (!sp)
        return;
    Panel *old = (*sp).content;
    if (old == content)
        return;
    Panel *self = &(*sp).base;
    if (old)
        Panel_removeChild(self, old);
    (*sp).content = nullptr;
    if (content && content != self) {
        Panel_addContainer(self, content);
        (*sp).content = content;
    }
    ScrollPanel_setOffset(sp, (*sp).offsetX, (*sp).offsetY);
}

void ScrollPanel_setOffset(ScrollPanel *sp, float x, float y) {
    if (!sp)
        return;
    float loX = 0.0f;
    float hiX = 0.0f;
    float loY = 0.0f;
    float hiY = 0.0f;
    offsetBounds(sp, &loX, &hiX, &loY, &hiY);
    float over = (*sp).overscroll > 0.0f ? (*sp).overscroll : 0.0f;
    (*sp).offsetX = pinOffset(x, loX - over, hiX + over);
    (*sp).offsetY = pinOffset(y, loY - over, hiY + over);
    markDirty(sp);
    ScrollPanel_syncToBar(sp);
}

void ScrollPanel_syncFromBar(ScrollPanel *sp) {
    if (!sp)
        return;
    ScrollBar *bar = (*sp).bar;
    if (!bar)
        return;
    float lo = 0.0f;
    float hi = 0.0f;
    offsetBounds(sp, nullptr, nullptr, &lo, &hi);
    float bmin = 0.0f;
    float bmax = 1.0f;
    ScrollBar_getRange(bar, &bmin, &bmax);
    float span = bmax - bmin;
    float t = 0.0f;
    if (span != 0.0f)
        t = pinOffset((ScrollBar_getValue(bar) - bmin) / span, 0.0f, 1.0f);
    float extent = hi - lo;
    (*sp).offsetY = lo + t * extent;
    markDirty(sp);
}

void ScrollPanel_syncToBar(ScrollPanel *sp) {
    if (!sp)
        return;
    ScrollBar *bar = (*sp).bar;
    if (!bar)
        return;
    float lo = 0.0f;
    float hi = 0.0f;
    offsetBounds(sp, nullptr, nullptr, &lo, &hi);
    float extent = hi - lo;
    float t = 0.0f;
    if (extent > 0.0f)
        t = ((*sp).offsetY - lo) / extent;
    float bmin = 0.0f;
    float bmax = 1.0f;
    ScrollBar_getRange(bar, &bmin, &bmax);
    ScrollBar_setValue(bar, bmin + t * (bmax - bmin));
}

// SCROLLBAR PART

static void applyBarVisible(ScrollPanel *sp) {
    if (!sp || !(*sp).bar)
        return;
    Panel *thumb = &(*(*sp).bar).base;
    Container *c = &(*thumb).base;
    Container_setVisible(c, (*sp).barVisible);
}

void ScrollPanel_scrollbar_setVisible(ScrollPanel *sp, bool visible) {
    if (!sp)
        return;
    (*sp).barVisible = visible;
    applyBarVisible(sp);
    markDirty(sp);
}

void ScrollPanel_scrollbar_setBar(ScrollPanel *sp, ScrollBar *bar) {
    if (!sp || !bar || (*sp).bar == bar)
        return;
    // Borrowed view, detach-only (Rule 29): old bar is detached, never
    // freed; the new one is attached, never re-owned. Games swap in a
    // skinned track+thumb and the offset sync keeps working untouched.
    Panel *self = &(*sp).base;
    if ((*sp).bar)
        Panel_removeChild(self, &(*(*sp).bar).base);
    Panel_addContainer(self, &(*bar).base);
    (*sp).bar = bar;
    applyBarVisible(sp);
    ScrollPanel_syncToBar(sp);
    markDirty(sp);
}

// FEEL PART

// Slippery 0..1 maps to exponential friction: 0 stops dead (legacy),
// 1 glides far. friction = 12 at 0, 0.8 at 1.
static float feelFriction(float slippery) {
    return 12.0f + (0.8f - 12.0f) * slippery;
}

void ScrollPanel_setSlippery(ScrollPanel *sp, float slippery) {
    if (!sp)
        return;
    if (slippery < 0.0f)
        slippery = 0.0f;
    if (slippery > 1.0f)
        slippery = 1.0f;
    (*sp).slippery = slippery;
}

void ScrollPanel_setOverscroll(ScrollPanel *sp, float px) {
    if (!sp)
        return;
    (*sp).overscroll = px > 0.0f ? px : 0.0f;
    ScrollPanel_setOffset(sp, (*sp).offsetX, (*sp).offsetY);
}

void ScrollPanel_fling(ScrollPanel *sp, float vx, float vy) {
    if (!sp)
        return;
    (*sp).velX = vx;
    (*sp).velY = vy;
    markDirty(sp);
}

void ScrollPanel_stop(ScrollPanel *sp) {
    if (!sp)
        return;
    (*sp).velX = 0.0f;
    (*sp).velY = 0.0f;
}

static float tickAxis(float off, float *vel, float lo, float hi, float over, float friction, double dt) {
    float v = *vel;
    if (v != 0.0f) {
        off += v * (float)dt;
        // Extra damping while riding the rubber band past the edge.
        bool past = off < lo || off > hi;
        float damp = friction * (float)dt;
        if (past)
            damp += 6.0f * (float)dt;
        v *= expf(-damp);
        if (fabsf(v) < 1.0f)
            v = 0.0f;
        // Hard stop at the rubber limit (no velocity bounce).
        if (off < lo - over) {
            off = lo - over;
            v = 0.0f;
        } else if (off > hi + over) {
            off = hi + over;
            v = 0.0f;
        }
    } else if (off < lo || off > hi) {
        // Spring home: exponential approach, snap when close.
        float bound = off < lo ? lo : hi;
        float pull = (bound - off) * (14.0f * (float)dt > 1.0f ? 1.0f : 14.0f * (float)dt);
        off += pull;
        if (fabsf(bound - off) < 0.5f)
            off = bound;
    }
    *vel = v;
    return off;
}

void ScrollPanel_tick(ScrollPanel *sp, double dt) {
    if (!sp || dt <= 0.0)
        return;
    float loX = 0.0f, hiX = 0.0f, loY = 0.0f, hiY = 0.0f;
    offsetBounds(sp, &loX, &hiX, &loY, &hiY);
    float over = (*sp).overscroll;
    float friction = feelFriction((*sp).slippery);
    bool wasMoving = (*sp).velX != 0.0f || (*sp).velY != 0.0f;
    bool wasOut = (*sp).offsetX < loX || (*sp).offsetX > hiX
        || (*sp).offsetY < loY || (*sp).offsetY > hiY;
    (*sp).offsetX = tickAxis((*sp).offsetX, &(*sp).velX, loX, hiX, over, friction, dt);
    (*sp).offsetY = tickAxis((*sp).offsetY, &(*sp).velY, loY, hiY, over, friction, dt);
    if (wasMoving || wasOut || (*sp).velX != 0.0f || (*sp).velY != 0.0f)
        markDirty(sp);
    ScrollPanel_syncToBar(sp);
}

// CONTENT-PANEL PART

void ScrollPanel_panel_setSize(ScrollPanel *sp, float w, float h) {
    if (!sp || !(*sp).content)
        return;
    Container_setSize(&(*(*sp).content).base, w, h);
    ScrollPanel_setOffset(sp, (*sp).offsetX, (*sp).offsetY);
}

void ScrollPanel_panel_setBackgroundColor(ScrollPanel *sp, uint32_t color) {
    if (!sp || !(*sp).content)
        return;
    Panel_setBackgroundColor((*sp).content, color);
}

void ScrollPanel_panel_setRadius(ScrollPanel *sp, float radius) {
    if (!sp || !(*sp).content)
        return;
    Panel_setRadius((*sp).content, radius);
}

// SETTERS

void ScrollPanel_setStartInset(ScrollPanel *sp, float inset) {
    if (!sp)
        return;
    (*sp).startInset = inset;
    ScrollPanel_setOffset(sp, (*sp).offsetX, (*sp).offsetY);
}

void ScrollPanel_setEndInset(ScrollPanel *sp, float inset) {
    if (!sp)
        return;
    (*sp).endInset = inset;
    ScrollPanel_setOffset(sp, (*sp).offsetX, (*sp).offsetY);
}

// GETTERS

Panel *ScrollPanel_getContent(const ScrollPanel *sp) {
    return sp ? (*sp).content : nullptr;
}

void ScrollPanel_getOffset(const ScrollPanel *sp, float *outX, float *outY) {
    float x = 0.0f;
    float y = 0.0f;
    if (sp) {
        x = (*sp).offsetX;
        y = (*sp).offsetY;
    }
    if (outX)
        (*outX) = x;
    if (outY)
        (*outY) = y;
}

float ScrollPanel_getStartInset(const ScrollPanel *sp) {
    return sp ? (*sp).startInset : 0.0f;
}

float ScrollPanel_getEndInset(const ScrollPanel *sp) {
    return sp ? (*sp).endInset : 0.0f;
}

ScrollBar *ScrollPanel_getBar(const ScrollPanel *sp) {
    return sp ? (*sp).bar : nullptr;
}

bool ScrollPanel_scrollbar_isVisible(const ScrollPanel *sp) {
    return sp && (*sp).barVisible;
}

float ScrollPanel_getSlippery(const ScrollPanel *sp) {
    return sp ? (*sp).slippery : 0.0f;
}

float ScrollPanel_getOverscroll(const ScrollPanel *sp) {
    return sp ? (*sp).overscroll : 0.0f;
}

void ScrollPanel_getVelocity(const ScrollPanel *sp, float *outVX, float *outVY) {
    float vx = 0.0f, vy = 0.0f;
    if (sp) {
        vx = (*sp).velX;
        vy = (*sp).velY;
    }
    if (outVX)
        (*outVX) = vx;
    if (outVY)
        (*outVY) = vy;
}

bool ScrollPanel_isScrolling(const ScrollPanel *sp) {
    return sp && ((*sp).velX != 0.0f || (*sp).velY != 0.0f);
}

bool ScrollPanel_isOverscrolled(const ScrollPanel *sp) {
    if (!sp)
        return false;
    float loX = 0.0f, hiX = 0.0f, loY = 0.0f, hiY = 0.0f;
    offsetBounds(sp, &loX, &hiX, &loY, &hiY);
    return (*sp).offsetX < loX || (*sp).offsetX > hiX
        || (*sp).offsetY < loY || (*sp).offsetY > hiY;
}

void ScrollPanel_panel_getSize(const ScrollPanel *sp, float *outW, float *outH) {
    float w = 0.0f, h = 0.0f;
    if (sp && (*sp).content) {
        w = Container_getWidth(&(*(*sp).content).base);
        h = Container_getHeight(&(*(*sp).content).base);
    }
    if (outW)
        (*outW) = w;
    if (outH)
        (*outH) = h;
}

uint32_t ScrollPanel_panel_getBackgroundColor(const ScrollPanel *sp) {
    if (sp && (*sp).content)
        return Panel_getBackgroundColor((*sp).content);
    return PANEL_COLOR_CLEAR;
}

float ScrollPanel_panel_getRadius(const ScrollPanel *sp) {
    if (sp && (*sp).content)
        return Panel_getRadius((*sp).content);
    return 0.0f;
}
