#ifndef DARLING_SCROLL_PANEL_H
#define DARLING_SCROLL_PANEL_H

#include <stdbool.h>
#include <stdint.h>

#include "c23/constructor.h"
#include "darling/field/scrollbar.h"
#include "darling/panel/panel.h"
#include "oop/type.h"

#ifndef ID_SCROLL_PANEL
#define ID_SCROLL_PANEL 0x006Du
#endif
#ifndef TYPE_SCROLL_PANEL_SINGLETON
#define TYPE_SCROLL_PANEL_SINGLETON (FORM_SINGLETON | ID_SCROLL_PANEL)
#endif

// darling/panel/scroll_panel.h — viewport over an oversized content panel.
//
// Three sub-APIs, one owner (never touch members directly):
//   ScrollPanel_*           — viewport core (content, offsets, insets)
//   ScrollPanel_scrollbar_* — the owned ScrollBar part (visibility)
//   ScrollPanel_panel_*     — the content Panel part (size, color, radius)
//
// Feel (touchscreen physics): fling velocity + slippery friction +
// overscroll rubber-band, advanced on Thread 0 via ScrollPanel_tick.

typedef struct ScrollPanel {
    // --- ScrollPanel core (owner fields: viewport state, not any part) ---
    Panel base;
    Panel *content;   // viewport child (edited via panel_* part); null = empty
    float offsetX;    // scroll offset into content (single source of truth)
    float offsetY;
    float startInset; // padding before the first child
    float endInset;   // overscroll past the last child
    // --- Scrollbar part (scrollbar_* verbs; bar is a live view) ---
    ScrollBar *bar;   // owned vertical bar (replaceable view, never freed)
    bool barVisible;  // scrollbar-part visibility (default true)
    // --- Feel part (owner fields: touchscreen physics, tick advances) ---
    float velX;       // fling velocity px/sec (decays in tick)
    float velY;
    float slippery;   // 0 stops dead, 1 long glide (default 0)
    float overscroll; // rubber-band px past each end (0 = disabled)
    // NOTE: the content-panel part (panel_* verbs) owns NO fields here —
    // it forwards to content above. New stored panel state is a smell.
} ScrollPanel;

// Constructors:
//   ScrollPanel(viewW, viewH)   — clipped viewport with an owned vertical bar
ScrollPanel *ScrollPanel_2(float viewW, float viewH);

#define ScrollPanel(...) CONSTRUCTOR_DISPATCH(ScrollPanel, __VA_ARGS__)

// Core (content attach is detach-only, never frees; bar<->offset stay synced).
void ScrollPanel_setContent(ScrollPanel *sp, Panel *content);
void ScrollPanel_setOffset(ScrollPanel *sp, float x, float y);
void ScrollPanel_syncFromBar(ScrollPanel *sp);
void ScrollPanel_syncToBar(ScrollPanel *sp);

// Setters.
void ScrollPanel_setStartInset(ScrollPanel *sp, float inset);
void ScrollPanel_setEndInset(ScrollPanel *sp, float inset);

// Scrollbar part (its own thing: visibility of the owned bar node).
void ScrollPanel_scrollbar_setVisible(ScrollPanel *sp, bool visible);
void ScrollPanel_scrollbar_setBar(ScrollPanel *sp, ScrollBar *bar);

// Feel part (touchscreen physics: fling + slippery + overscroll).
void ScrollPanel_setSlippery(ScrollPanel *sp, float slippery);
void ScrollPanel_setOverscroll(ScrollPanel *sp, float px);
void ScrollPanel_fling(ScrollPanel *sp, float vx, float vy);
void ScrollPanel_stop(ScrollPanel *sp);
void ScrollPanel_tick(ScrollPanel *sp, double dt);

// Content-panel part (modify the panel through here, never panel->field).
void ScrollPanel_panel_setSize(ScrollPanel *sp, float w, float h);
void ScrollPanel_panel_setBackgroundColor(ScrollPanel *sp, uint32_t color);
void ScrollPanel_panel_setRadius(ScrollPanel *sp, float radius);

// Getters.
Panel *ScrollPanel_getContent(const ScrollPanel *sp);
void ScrollPanel_getOffset(const ScrollPanel *sp, float *outX, float *outY);
float ScrollPanel_getStartInset(const ScrollPanel *sp);
float ScrollPanel_getEndInset(const ScrollPanel *sp);
ScrollBar *ScrollPanel_getBar(const ScrollPanel *sp);

// Scrollbar-part getters.
bool ScrollPanel_scrollbar_isVisible(const ScrollPanel *sp);

// Feel-part getters.
float ScrollPanel_getSlippery(const ScrollPanel *sp);
float ScrollPanel_getOverscroll(const ScrollPanel *sp);
void ScrollPanel_getVelocity(const ScrollPanel *sp, float *outVX, float *outVY);
bool ScrollPanel_isScrolling(const ScrollPanel *sp);
bool ScrollPanel_isOverscrolled(const ScrollPanel *sp);

// Content-panel-part getters.
void ScrollPanel_panel_getSize(const ScrollPanel *sp, float *outW, float *outH);
uint32_t ScrollPanel_panel_getBackgroundColor(const ScrollPanel *sp);
float ScrollPanel_panel_getRadius(const ScrollPanel *sp);

#endif
