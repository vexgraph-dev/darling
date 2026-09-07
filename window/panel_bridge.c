// Include our headers BEFORE any ObjC to avoid CarbonCore's `Collection`
// typedef colliding with our struct Collection (collection.h).
#include <stddef.h>
#include <stdatomic.h>
#include "oop/type.h"
#include "nio/mem.h"
#include "lang/vec4.h"
#include "darling/container.h"
#include "darling/panel/panel.h"
#include "darling/panel/scroll_panel.h"
#include "window/window.h"
#include "annotation/overview.h"

;;OVERVIEW
/**
 * ============================================================================
 * MODULE: Panel_bridge (window/panel_bridge.c)
 * LEVEL: L4 — Self-Management (OS window/IOSurface panel glue)
 * ============================================================================
 * pure-C bridge for IOSurface panel operations.
 *
 * STRUCT FIELDS: none — procedural (pure-C IOSurface panel glue, no struct).
 *
 * FUNCTION REGISTRY:
 * ----------------------------------------------------------------------------
 * Constructors:
 *   - PanelCocoa_new(panel, w, h)
 *
 * Core Functions:
 *   - anti_AttachPanelIOSurfaceChildren(window, contentPanel, width, height)
 *   - TextCore_backingScale(void)
 *   - for(i++)
 *   - PanelCocoa_fromPanel(panel)
 *   - anti_ResizePanelIOSurfaceChildren(window, contentPanel, width, height)
 *   - anti_CompositeIOSurfaceChildren(window, contentPanel)
 *
 * Setters:
 *   - PanelCocoa_setSize(pc, w, h)
 *
 * Getters:
 *   - anti_GetPanelMaxSize(p, outMaxW, outMaxH)
 *   - anti_GetChildLayout(child, winW, winH, outX, outY, outW, outH)
 *   - anti_GetChildCount(contentPanel)
 *   - anti_GetChildAt(contentPanel, index)
 *   - Panel_getChild(contentPanel, index)
 *   - anti_GetPanelSize(p, outW, outH)
 *   - anti_GetChildParentAnchor(child)
 *   - anti_GetChildSelfAnchor(child)
 * ============================================================================
 */


// src/window/panel_bridge.c — pure-C bridge for IOSurface panel operations.
//
// LAYER MODEL (the stack, front to back):
//   scrollbar ......... own IOSurface (ScrollPanel right-dock, topmost child)
//   1st-gen children .. one IOSurface EACH (attached here, per direct child)
//   window ............ CAMetalLayer (AppKit-owned, Vulkan never touches it)
//
// Darling numbering:
//   layer 1 = window (CAMetalLayer, the composite target)
//   layer 2 = contentPanel — most honestly a SCROLLPANEL: the window IS y,
//             not a child filling it. Children a/b/c/d sit inside y under
//             ABSOLUTE layout (explicit frames, no layout manager).
//   layer 3 = first-gen panels (own IOSurface each; deeper nesting —
//             a1/a2/a3 inside a — paints inside the parent surface via
//             Vulkan render handlers (Panel_setRenderHandler, cf. vk_test's
//             hud_pulse/pic_render), NOT as nested layers/JPanels).
//   scrollbar = own IOSurface at the layer-3 edge (topmost child order).
//   Scroll offsets reach layers through ScrollPanel_childFrame (content
//   shifts by -offset, chrome stays) — C-side resolve is the source of
//   truth, exactly as vk_test's hand-placed anchors proved.
//
// TRAFFIC LAW: Vulkan renders INSIDE the IOSurfaces (producer only) and
// never presents the window; WindowServer composites the stack onto the
// CAMetalLayer. Resize = layer moves via anchors, zero repaint.
//
// Each content panel child gets an IOSurface. Vulkan renders into each
// IOSurface independently (async). AppKit composites the CALayers.
// This file iterates children and calls into ObjC PanelCocoa for the
// IOSurface/CALayer plumbing.

// Attach IOSurface backing to ALL children of a content panel.
// Returns the number of IOSurface backings attached.
int anti_AttachPanelIOSurfaceChildren(Window *window, Panel *contentPanel, int width, int height) {
    if (!window || !contentPanel)
        return 0;
    (void) window;
    int attached = 0;
    size_t childCount = Panel_childCount(contentPanel);
    extern float TextCore_backingScale(void);
    float scale = TextCore_backingScale();
    if (scale <= 0.0f)
        scale = 1.0f;

    for (size_t i = 0; i < childCount; i++) {
        Panel *child = Panel_getChild(contentPanel, i);
        if (!child)
            continue;

        // Skip completely transparent panels with no render handler and no scene.
        // Pure clamp/layout spacers (like ScrollPanel bounds) must never own an IOSurface.
        uint32_t bg = Panel_getBackgroundColor(child);
        Panel_RenderFn rfn = Panel_getRenderHandler(child);
        uint64_t childType = Memory_type(child);
        bool isScene = (childType == TYPE_SCENE3D_SINGLETON || childType == TYPE_SCENE2D_SINGLETON
                        || childType == TYPE_SCENE_SINGLETON);
        if (bg == PANEL_COLOR_CLEAR && !rfn && !isScene)
            continue;

        // Get the child's MAX size for IOSurface allocation (fixed, never reallocates)
        int maxW = 0, maxH = 0;
        extern void anti_GetPanelMaxSize(Panel *p, int *outMaxW, int *outMaxH);
        anti_GetPanelMaxSize(child, &maxW, &maxH);
        if (maxW <= 0 || maxH <= 0) {
            // Fallback: use layout rect
            Vec4 rect;
            Container_resolve(&(*child).base, 0.0f, 0.0f, (float) width, (float) height, &rect);
            maxW = (int) (rect.z + 0.5f);
            maxH = (int) (rect.w + 0.5f);
        }
        if (maxW <= 0 || maxH <= 0)
            continue;

        int allocW = (int) (maxW * scale + 0.5f);
        int allocH = (int) (maxH * scale + 0.5f);
        if (allocW <= 0 || allocH <= 0 || allocW > 16384 || allocH > 16384)
            continue;

        // Check if already attached
        extern void *PanelCocoa_fromPanel(void *panel);
        void *pc = PanelCocoa_fromPanel(child);
        if (pc) {
            extern bool PanelCocoa_setSize(void *pc, int w, int h);
            PanelCocoa_setSize(pc, allocW, allocH);
        } else {
            extern void *PanelCocoa_new(void *panel, int w, int h);
            if (PanelCocoa_new(child, allocW, allocH))
                attached++;
        }
    }
    return attached;
}

// Resize IOSurface backing for ALL children of a content panel.
// Returns the number of IOSurface backings resized.
int anti_ResizePanelIOSurfaceChildren(Window *window, Panel *contentPanel, int width, int height) {
    if (!window || !contentPanel)
        return 0;
    (void) window;
    int resized = 0;
    size_t childCount = Panel_childCount(contentPanel);
    extern float TextCore_backingScale(void);
    float scale = TextCore_backingScale();
    if (scale <= 0.0f)
        scale = 1.0f;

    for (size_t i = 0; i < childCount; i++) {
        Panel *child = Panel_getChild(contentPanel, i);
        if (!child)
            continue;
        extern void *PanelCocoa_fromPanel(void *panel);
        void *pc = PanelCocoa_fromPanel(child);
        if (pc) {
            Vec4 rect;
            Container_resolve(&(*child).base, 0.0f, 0.0f, (float) width, (float) height, &rect);
            int w = (int) (rect.z * scale + 0.5f);
            int h = (int) (rect.w * scale + 0.5f);
            if (w > 16384) w = 16384;
            if (h > 16384) h = 16384;
            if (w > 0 && h > 0) {
                extern bool PanelCocoa_setSize(void *pc, int w, int h);
                PanelCocoa_setSize(pc, w, h);
                resized++;
            }
        }
    }
    return resized;
}

// Composite IOSurface-backed children into the window's layer tree.
void anti_CompositeIOSurfaceChildren(Window *window, Panel *contentPanel) {
    (void)window;
    (void)contentPanel;
    // Real implementation is in window_cocoa.m (ObjC)
}

// Layout helper for ObjC side (avoids pulling darling/panel.h into ObjC).
// C-side resolve is the source of truth: when the child's parent is a
// ScrollPanel (the window-IS-y model), the frame goes through
// ScrollPanel_childFrame so content shifts by -offset while chrome (the
// bar) stays — raw Container_resolve would pin scrolled content in place.
void anti_GetChildLayout(Panel *child, float winW, float winH, float *outX, float *outY, float *outW, float *outH) {
    if (!child || !outX || !outY || !outW || !outH) return;
    Panel *parent = Panel_getParent(child);
    if (parent && Memory_type(parent) == TYPE_SCROLL_PANEL_SINGLETON) {
        ScrollPanel_childFrame((ScrollPanel *)parent, child, winW, winH, outX, outY, outW, outH);
        return;
    }
    Vec4 rect;
    Container_resolve(&(*child).base, 0.0f, 0.0f, winW, winH, &rect);
    *outX = rect.x;
    *outY = rect.y;
    *outW = rect.z;
    *outH = rect.w;
}

// Child iteration helpers for ObjC side (avoids pulling panel.h into ObjC).
int anti_GetChildCount(Panel *contentPanel) {
    if (!contentPanel) return 0;
    return (int)Panel_childCount(contentPanel);
}

Panel *anti_GetChildAt(Panel *contentPanel, int index) {
    if (!contentPanel) return nullptr;
    return Panel_getChild(contentPanel, index);
}

// Get panel's max size (first setSize = max, subsequent = clamped current).
void anti_GetPanelMaxSize(Panel *p, int *outMaxW, int *outMaxH) {
    if (!p || !outMaxW || !outMaxH) return;
    *outMaxW = (int)((*p).base.maxW + 0.5f);
    *outMaxH = (int)((*p).base.maxH + 0.5f);
}

// Get panel's current display size.
void anti_GetPanelSize(Panel *p, int *outW, int *outH) {
    if (!p || !outW || !outH) return;
    *outW = (int)((*p).base.w + 0.5f);
    *outH = (int)((*p).base.h + 0.5f);
}

int anti_GetChildParentAnchor(Panel *child) {
    if (!child) return CONTAINER_PARENT_ANCHOR_TOP_LEFT;
    return Container_getParentAnchor(&(*child).base);
}

int anti_GetChildSelfAnchor(Panel *child) {
    if (!child) return CONTAINER_SELF_ANCHOR_TOP_LEFT;
    return Container_getSelfAnchor(&(*child).base);
}
