#include "darling/field/textarea.h"

#include <string.h>

#include "annotation/incomplete.h"
#include "nio/mem.h"
#include "oop/type.h"
#include "annotation/overview.h"

;;OVERVIEW
/**
 * ============================================================================
 * CLASS: Textarea (inherits Panel, LEVEL L2 Behavior)
 * ============================================================================
 * Multi-line text area shell: Panel layout plus an owned text buffer with
 * visible-line count, word-wrap mode, and vertical scroll offset.
 *
 * STRUCT FIELDS (Mirroring darling/field/textarea.h):
 * ----------------------------------------------------------------------------
 *   Panel base;            // Inherited layout, bounds, and hierarchy state
 *   char *text;            // Owned UTF-8 buffer
 *   int32_t visibleLines;  // Viewport height in lines
 *   int32_t wrap;          // Wrap mode (0 = off, 1 = word)
 *   float scrollY;         // Vertical scroll offset in points
 *   Font *font;            // Optional SDF font descriptor (borrowed)
 *
 * FUNCTION REGISTRY:
 * ----------------------------------------------------------------------------
 * Constructors:
 *   - Textarea()                     : Textarea_0()
 *   - Textarea(parent, visibleLines) : Textarea_2(parent, visibleLines)
 *
 * Core Functions:
 *   - Textarea_scrollTo(ta, y)
 *
 * Setters:
 *   - Textarea_setText(ta, text)
 *   - Textarea_setVisibleLines(ta, lines)
 *   - Textarea_setWrap(ta, wrap)
 *   - Textarea_setScrollY(ta, y)
 *   - Textarea_setFont(ta, font)
 *   - Textarea_free(ta)
 *
 * Getters:
 *   - Textarea_getText(ta)
 *   - Textarea_getVisibleLines(ta)
 *   - Textarea_getWrap(ta)
 *   - Textarea_getScrollY(ta)
 *   - Textarea_getFont(ta)
 * ============================================================================
 */

// ============================================================================
// CONSTRUCTORS
// ============================================================================

#define TEXTAREA_DEFAULT_LINES 4

Textarea *Textarea_0(void) {
    Textarea *ta = (Textarea*) Memory_alloc(TYPE_TEXTAREA_SINGLETON, sizeof(Textarea));
    if (!ta)
        return nullptr;
    Panel *bp = Panel_0();
    if (!bp) {
        Memory_free(ta);
        return nullptr;
    }
    (*ta).base = (*bp);
    Memory_free(bp);
    (*ta).text = nullptr;
    (*ta).visibleLines = TEXTAREA_DEFAULT_LINES;
    (*ta).wrap = 1;
    (*ta).scrollY = 0.0f;
    (*ta).font = nullptr;
    return ta;
}

Textarea *Textarea_2(Panel *parent, int32_t visibleLines) {
    Textarea *ta = Textarea_0();
    if (!ta)
        return nullptr;
    (*ta).visibleLines = visibleLines;
    if (parent) {
        Panel *bp = &(*ta).base;
        Panel_addContainer(parent, bp);
    }
    return ta;
}

// ============================================================================
// CORE FUNCTIONS
// ============================================================================

void Textarea_scrollTo(Textarea *ta, float y) {
    ;;INCOMPLETE // clamped scroll lands with the scroll walker
    (void)ta;
    (void)y;
}

// ============================================================================
// SETTERS
// ============================================================================

static void markDirty(Textarea *ta) {
    if (!ta)
        return;
    Panel *bp = &(*ta).base;
    Container_markDirty(&(*bp).base);
}

void Textarea_setText(Textarea *ta, const char *text) {
    if (!ta)
        return;
    char *old = (*ta).text;
    if (old) {
        Memory_free(old);
        (*ta).text = nullptr;
    }
    if (text) {
        size_t len = strlen(text) + 1;
        char *buf = (char*) Memory_alloc(TYPE_ARRAY, len);
        if (buf)
            memcpy(buf, text, len);
        (*ta).text = buf;
    }
    markDirty(ta);
}

void Textarea_setVisibleLines(Textarea *ta, int32_t lines) {
    if (!ta)
        return;
    (*ta).visibleLines = lines;
    markDirty(ta);
}

void Textarea_setWrap(Textarea *ta, int32_t wrap) {
    if (!ta)
        return;
    (*ta).wrap = wrap;
    markDirty(ta);
}

void Textarea_setScrollY(Textarea *ta, float y) {
    if (!ta)
        return;
    (*ta).scrollY = y;
    markDirty(ta);
}

void Textarea_setFont(Textarea *ta, Font *font) {
    if (!ta)
        return;
    (*ta).font = font;
    markDirty(ta);
}

void Textarea_free(Textarea *ta) {
    if (!ta)
        return;
    char *text = (*ta).text;
    if (text)
        Memory_free(text);
    (*ta).text = nullptr;
    (*ta).font = nullptr;
    Memory_free(ta);
}

// ============================================================================
// GETTERS
// ============================================================================

const char *Textarea_getText(const Textarea *ta) {
    return ta ? (*ta).text : nullptr;
}

int32_t Textarea_getVisibleLines(const Textarea *ta) {
    return ta ? (*ta).visibleLines : 0;
}

int32_t Textarea_getWrap(const Textarea *ta) {
    return ta ? (*ta).wrap : 0;
}

float Textarea_getScrollY(const Textarea *ta) {
    return ta ? (*ta).scrollY : 0.0f;
}

Font *Textarea_getFont(const Textarea *ta) {
    return ta ? (*ta).font : nullptr;
}
