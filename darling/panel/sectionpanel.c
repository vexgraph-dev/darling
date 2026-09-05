#include "darling/panel/sectionpanel.h"

#include "darling/panel/panel.h"
#include "annotation/incomplete.h"
#include "annotation/overview.h"
#include "nio/mem.h"
#include "oop/type.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

;;OVERVIEW
/**
 * ============================================================================
 * CLASS: SectionPanel (embeds Panel)
 * LEVEL: L2 — Behavior (section-switching panel shell)
 * ============================================================================
 * Panel shell whose children are sections with exactly one current index.
 * Selection advance clamps or wraps today; showing/hiding section children
 * lands in a later pass.
 *
 * STRUCT FIELDS (Mirroring darling/panel/sectionpanel.h):
 * ----------------------------------------------------------------------------
 *   Panel base;                        // Inherited layout/tree/background state
 *   int32_t current;                   // Current section index
 *   bool wrapAround;                   // True = next/prev wraps at the ends
 *   void (*onSectionChange)(void *ctx);// Change callback; nullptr = none
 *   void *ctx;                         // Callback context
 *
 * FUNCTION REGISTRY:
 * ----------------------------------------------------------------------------
 * Constructors:
 *   - SectionPanel_0(void)
 *   - SectionPanel_1(parent)
 *
 * Core Functions:
 *   - SectionPanel_next(s)
 *   - SectionPanel_prev(s)
 *
 * Setters:
 *   - SectionPanel_setCurrent(s, index)
 *   - SectionPanel_setWrapAround(s, wrap)
 *
 * Getters:
 *   - SectionPanel_getCurrent(s)
 *   - SectionPanel_getCount(s)
 *   - SectionPanel_getWrapAround(s)
 * ============================================================================
 */

// CONSTRUCTORS
// ============================================================================

SectionPanel *SectionPanel_0(void) {
    SectionPanel *s = (SectionPanel*) Memory_alloc(TYPE_SECTION_PANEL_SINGLETON, sizeof(SectionPanel));
    if (!s)
        return nullptr;
    Panel *b = Panel_0();
    if (!b) {
        Memory_free(s);
        return nullptr;
    }
    (*s).base = (*b);
    Memory_free(b);
    (*s).current = 0;
    (*s).wrapAround = false;
    (*s).onSectionChange = nullptr;
    (*s).ctx = nullptr;
    return s;
}

SectionPanel *SectionPanel_1(Panel *parent) {
    SectionPanel *s = SectionPanel_0();
    if (s && parent) {
        Panel *b = &(*s).base;
        Panel_addContainer(parent, b);
    }
    return s;
}

// CORE FUNCTIONS
// ============================================================================

void SectionPanel_next(SectionPanel *s) {
    ;;INCOMPLETE // full show/hide of section children deferred
    if (!s)
        return;
    SectionPanel_setCurrent(s, (*s).current + 1);
}

void SectionPanel_prev(SectionPanel *s) {
    ;;INCOMPLETE // full show/hide of section children deferred
    if (!s)
        return;
    SectionPanel_setCurrent(s, (*s).current - 1);
}

// SETTERS
// ============================================================================

static void markDirty(SectionPanel *s) {
    if (!s)
        return;
    Panel *b = &(*s).base;
    Container *c = &(*b).base;
    Container_markDirty(c);
}

void SectionPanel_setCurrent(SectionPanel *s, int32_t index) {
    if (!s)
        return;
    Panel *b = &(*s).base;
    size_t n = Panel_childCount(b);
    if (n == 0) {
        (*s).current = 0;
        markDirty(s);
        return;
    }
    int32_t count = (int32_t) n;
    if ((*s).wrapAround) {
        int32_t m = index % count;
        if (m < 0)
            m += count;
        (*s).current = m;
    } else {
        if (index < 0)
            index = 0;
        if (index >= count)
            index = count - 1;
        (*s).current = index;
    }
    markDirty(s);
}

void SectionPanel_setWrapAround(SectionPanel *s, bool wrap) {
    if (!s)
        return;
    (*s).wrapAround = wrap;
    markDirty(s);
}

// GETTERS
// ============================================================================

int32_t SectionPanel_getCurrent(const SectionPanel *s) {
    return s ? (*s).current : 0;
}

int32_t SectionPanel_getCount(const SectionPanel *s) {
    if (!s)
        return 0;
    const Panel *b = &(*s).base;
    return (int32_t) Panel_childCount(b);
}

bool SectionPanel_getWrapAround(const SectionPanel *s) {
    return s ? (*s).wrapAround : false;
}
