#include "darling/panel/layeredpanel.h"

#include "darling/panel/panel.h"
#include "annotation/overview.h"
#include "nio/mem.h"
#include "oop/type.h"

#include <stdbool.h>
#include <stdint.h>

;;OVERVIEW
/**
 * ============================================================================
 * CLASS: LayeredPanel (embeds Panel)
 * LEVEL: L2 — Behavior (stacked-band panel shell)
 * ============================================================================
 * Panel shell that stacks a fixed number of bands with a per-band visibility
 * mask. Band painting and hit routing land in a later pass.
 *
 * STRUCT FIELDS (Mirroring darling/panel/layeredpanel.h):
 * ----------------------------------------------------------------------------
 *   Panel base;               // Inherited layout/tree/background state
 *   int32_t bandCount;        // Number of stacked bands (>= 0)
 *   uint32_t bandVisibleMask; // Bit i set = band i visible (bands 0..31)
 *
 * FUNCTION REGISTRY:
 * ----------------------------------------------------------------------------
 * Constructors:
 *   - LayeredPanel_0(void)
 *   - LayeredPanel_1(parent)
 *
 * Core Functions:
 *   - (none — shell; band paint and hit routing deferred)
 *
 * Setters:
 *   - LayeredPanel_setBandCount(p, count)
 *   - LayeredPanel_setBandVisible(p, band, visible)
 *
 * Getters:
 *   - LayeredPanel_getBandCount(p)
 *   - LayeredPanel_isBandVisible(p, band)
 * ============================================================================
 */

// CONSTRUCTORS
// ============================================================================

LayeredPanel *LayeredPanel_0(void) {
    LayeredPanel *p = (LayeredPanel*) Memory_alloc(TYPE_LAYERED_PANEL_SINGLETON, sizeof(LayeredPanel));
    if (!p)
        return nullptr;
    Panel *b = Panel_0();
    if (!b) {
        Memory_free(p);
        return nullptr;
    }
    (*p).base = (*b);
    Memory_free(b);
    (*p).bandCount = 0;
    (*p).bandVisibleMask = 0u;
    return p;
}

LayeredPanel *LayeredPanel_1(Panel *parent) {
    LayeredPanel *p = LayeredPanel_0();
    if (p && parent) {
        Panel *b = &(*p).base;
        Panel_addContainer(parent, b);
    }
    return p;
}

// CORE FUNCTIONS
// ============================================================================

// (none — shell; band paint and hit routing deferred)

// SETTERS
// ============================================================================

static void markDirty(LayeredPanel *p) {
    if (!p)
        return;
    Panel *b = &(*p).base;
    Container *c = &(*b).base;
    Container_markDirty(c);
}

void LayeredPanel_setBandCount(LayeredPanel *p, int32_t count) {
    if (!p)
        return;
    if (count < 0)
        count = 0;
    (*p).bandCount = count;
    markDirty(p);
}

void LayeredPanel_setBandVisible(LayeredPanel *p, int32_t band, bool visible) {
    if (!p)
        return;
    if (band < 0 || band >= 32)
        return;
    uint32_t bit = 1u << (uint32_t) band;
    if (visible)
        (*p).bandVisibleMask |= bit;
    else
        (*p).bandVisibleMask &= ~bit;
    markDirty(p);
}

// GETTERS
// ============================================================================

int32_t LayeredPanel_getBandCount(const LayeredPanel *p) {
    return p ? (*p).bandCount : 0;
}

bool LayeredPanel_isBandVisible(const LayeredPanel *p, int32_t band) {
    if (!p)
        return false;
    if (band < 0 || band >= 32)
        return false;
    uint32_t bit = 1u << (uint32_t) band;
    return ((*p).bandVisibleMask & bit) != 0u;
}
