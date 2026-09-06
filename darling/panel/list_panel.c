#include "darling/panel/list_panel.h"

#include "annotation/overview.h"
#include "darling/panel/panel.h"
#include "nio/mem.h"
#include "oop/type.h"
#include "struct/list.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

;;OVERVIEW
/**
 * ============================================================================
 * CLASS: ListPanel (embeds Panel)
 * LEVEL: L2 — Behavior (indexed panel-stack layout behavior API)
 * ============================================================================
 * A vertical (or horizontal) stack owning ordered children where index IS
 * the API: append/insert/get/remove by index. Text bubbles, chat logs,
 * file rows, settings groups. Children stay ordinary Panels in the
 * embedded base's child list — no second child list. Detach-only: the
 * list never frees children. Every mutation re-runs the layout pass:
 * children stack along the axis with spacing; cross-axis each child
 * keeps its size and the list wraps the widest child, unless fillCross
 * stretches children to the list's own cross size.
 *
 * STRUCT FIELDS (Mirroring darling/panel/list_panel.h):
 * ----------------------------------------------------------------------------
 *   Panel base;          // Inherited layout/tree/background state; children
 *                        // live in (*base).children via Panel_* tree API
 *   int32_t direction;   // LIST_PANEL_VERTICAL (0) or LIST_PANEL_HORIZONTAL (1)
 *   float spacing;       // Gap between adjacent children in parent units (>= 0)
 *   bool fillCross;      // True = stretch children across the cross axis
 *                        // to the list's own cross size; false = wrap widest
 *
 * FUNCTION REGISTRY:
 * ----------------------------------------------------------------------------
 * Constructors:
 *   - ListPanel_0(void)
 *   - ListPanel_1(direction)
 *
 * Core Functions:
 *   - ListPanel_add(lp, child)
 *   - ListPanel_insert(lp, index, child)
 *   - ListPanel_get(lp, index)
 *   - ListPanel_remove(lp, index)
 *   - ListPanel_count(lp)
 *   - ListPanel_layout(lp)
 *
 * Setters:
 *   - ListPanel_setLocation(lp, x, y)
 *   - ListPanel_setSize(lp, w, h)
 *   - ListPanel_setSpacing(lp, spacing)
 *   - ListPanel_setDirection(lp, direction)
 *   - ListPanel_setFillCross(lp, fill)
 *
 * Getters:
 *   - ListPanel_getSpacing(lp)
 *   - ListPanel_getDirection(lp)
 *   - ListPanel_isFillCross(lp)
 * ============================================================================
 */

// CONSTRUCTORS
// ============================================================================

ListPanel *ListPanel_0(void) {
    ListPanel *lp = (ListPanel*) Memory_alloc(TYPE_LIST_PANEL_SINGLETON, sizeof(ListPanel));
    if (!lp)
        return nullptr;
    Panel *b = Panel_0();
    if (!b) {
        Memory_free(lp);
        return nullptr;
    }
    (*lp).base = (*b);
    Memory_free(b);
    (*lp).direction = LIST_PANEL_VERTICAL;
    (*lp).spacing = 0.0f;
    (*lp).fillCross = false;
    return lp;
}

ListPanel *ListPanel_1(int32_t direction) {
    ListPanel *lp = ListPanel_0();
    if (lp)
        ListPanel_setDirection(lp, direction);
    return lp;
}

// CORE FUNCTIONS
// ============================================================================

static void markDirty(ListPanel *lp) {
    if (!lp)
        return;
    Panel *b = &(*lp).base;
    Container *c = &(*b).base;
    Container_markDirty(c);
}

static void layoutVertical(Panel *b, Container *c, size_t n, float spacing, bool fill) {
    float selfW = Container_getWidth(c);
    float cursor = 0.0f;
    float maxW = 0.0f;
    size_t placed = 0;
    for (size_t i = 0; i < n; i++) {
        Panel *kid = Panel_getChild(b, i);
        if (!kid)
            continue;
        Container *kb = &(*kid).base;
        float kw = Container_getWidth(kb);
        float kh = Container_getHeight(kb);
        if (kw > maxW)
            maxW = kw;
        if (fill)
            Container_setWidth(kb, selfW);
        Container_setLocation(kb, 0.0f, cursor);
        cursor += kh + spacing;
        placed++;
    }
    if (placed == 0)
        return;
    if (!fill)
        Container_setWidth(c, maxW);
    Container_setHeight(c, cursor - spacing);
}

static void layoutHorizontal(Panel *b, Container *c, size_t n, float spacing, bool fill) {
    float selfH = Container_getHeight(c);
    float cursor = 0.0f;
    float maxH = 0.0f;
    size_t placed = 0;
    for (size_t i = 0; i < n; i++) {
        Panel *kid = Panel_getChild(b, i);
        if (!kid)
            continue;
        Container *kb = &(*kid).base;
        float kw = Container_getWidth(kb);
        float kh = Container_getHeight(kb);
        if (kh > maxH)
            maxH = kh;
        if (fill)
            Container_setHeight(kb, selfH);
        Container_setLocation(kb, cursor, 0.0f);
        cursor += kw + spacing;
        placed++;
    }
    if (placed == 0)
        return;
    Container_setWidth(c, cursor - spacing);
    if (!fill)
        Container_setHeight(c, maxH);
}

void ListPanel_layout(ListPanel *lp) {
    if (!lp)
        return;
    Panel *b = &(*lp).base;
    Container *c = &(*b).base;
    Container_markDirty(c);
    size_t n = Panel_childCount(b);
    if (n == 0)
        return;
    int32_t dir = (*lp).direction;
    float spacing = (*lp).spacing;
    bool fill = (*lp).fillCross;
    if (dir == LIST_PANEL_HORIZONTAL)
        layoutHorizontal(b, c, n, spacing, fill);
    else
        layoutVertical(b, c, n, spacing, fill);
    Container_markDirty(c);
}

void ListPanel_add(ListPanel *lp, Panel *child) {
    if (!lp || !child)
        return;
    Panel *b = &(*lp).base;
    if (child == b)
        return;
    Panel_addContainer(b, child);
    ListPanel_layout(lp);
}

void ListPanel_insert(ListPanel *lp, int32_t index, Panel *child) {
    if (!lp || !child)
        return;
    Panel *b = &(*lp).base;
    if (child == b)
        return;
    if (Panel_containsChild(b, child))
        Panel_removeChild(b, child);
    size_t n = Panel_childCount(b);
    if (index < 0)
        index = 0;
    if ((size_t) index > n)
        index = (int32_t) n;
    Panel_addContainer(b, child);
    if (!Panel_containsChild(b, child))
        return;
    if ((size_t) index < n) {
        List *kids = (*b).children;
        if (kids) {
            size_t at = (size_t) index;
            for (size_t j = n; j > at; j--)
                List_set(kids, j, List_get(kids, j - 1));
            List_set(kids, at, (uint64_t)(uintptr_t) child);
        }
    }
    ListPanel_layout(lp);
}

Panel *ListPanel_get(const ListPanel *lp, int32_t index) {
    if (!lp || index < 0)
        return nullptr;
    const Panel *b = &(*lp).base;
    return Panel_getChild(b, (size_t) index);
}

bool ListPanel_remove(ListPanel *lp, int32_t index) {
    if (!lp || index < 0)
        return false;
    Panel *b = &(*lp).base;
    Panel *kid = Panel_getChild(b, (size_t) index);
    if (!kid)
        return false;
    bool out = Panel_removeChild(b, kid);
    ListPanel_layout(lp);
    return out;
}

size_t ListPanel_count(const ListPanel *lp) {
    if (!lp)
        return 0;
    const Panel *b = &(*lp).base;
    return Panel_childCount(b);
}

// SETTERS
// ============================================================================

void ListPanel_setSpacing(ListPanel *lp, float spacing) {
    if (!lp)
        return;
    if (spacing < 0.0f)
        spacing = 0.0f;
    (*lp).spacing = spacing;
    ListPanel_layout(lp);
    markDirty(lp);
}

void ListPanel_setDirection(ListPanel *lp, int32_t direction) {
    if (!lp)
        return;
    if (direction != LIST_PANEL_VERTICAL && direction != LIST_PANEL_HORIZONTAL)
        return;
    (*lp).direction = direction;
    ListPanel_layout(lp);
    markDirty(lp);
}

void ListPanel_setFillCross(ListPanel *lp, bool fill) {
    if (!lp)
        return;
    (*lp).fillCross = fill;
    ListPanel_layout(lp);
    markDirty(lp);
}

// GETTERS
// ============================================================================

float ListPanel_getSpacing(const ListPanel *lp) {
    return lp ? (*lp).spacing : 0.0f;
}

int32_t ListPanel_getDirection(const ListPanel *lp) {
    return lp ? (*lp).direction : LIST_PANEL_VERTICAL;
}

bool ListPanel_isFillCross(const ListPanel *lp) {
    return lp ? (*lp).fillCross : false;
}
