#include "darling/field/radiogroup.h"

#include "annotation/incomplete.h"
#include "annotation/overview.h"
#include "nio/mem.h"
#include "oop/type.h"

;;OVERVIEW
/**
 * ============================================================================
 * CLASS: RadioGroup (embeds Panel)
 * LEVEL: L2 — Behavior (single-choice option behavior API)
 * ============================================================================
 * Single-choice option list holding owned label strings and a select hook.
 *
 * STRUCT FIELDS (Mirroring darling/field/radiogroup.h):
 * ----------------------------------------------------------------------------
 *   Panel base;                  // Inherited layout, bounds, hierarchy state
 *   List *options;               // Owned C strings; nullptr means empty
 *   int32_t selected;            // Active option index; -1 means none
 *   int32_t orientation;         // 0 means vertical, 1 means horizontal
 *   void (*onSelect)(void *ctx); // Select callback; nullptr means none
 *   void *ctx;                   // Callback context pointer
 *
 * FUNCTION REGISTRY:
 * ----------------------------------------------------------------------------
 * Constructors:
 *   - RadioGroup_0(void)
 *   - RadioGroup_1(parent)
 *
 * Core Functions:
 *   - RadioGroup_addOption(g, option)
 *   - RadioGroup_clear(g)
 *
 * Setters:
 *   - RadioGroup_setSelected(g, index)
 *   - RadioGroup_setOrientation(g, orientation)
 *   - RadioGroup_setOnSelect(g, cb)
 *   - RadioGroup_setCtx(g, ctx)
 *   - RadioGroup_free(g)
 *
 * Getters:
 *   - RadioGroup_getSelected(g)
 *   - RadioGroup_getOrientation(g)
 *   - RadioGroup_optionCount(g)
 *   - RadioGroup_getOption(g, index)
 *   - RadioGroup_getOnSelect(g)
 *   - RadioGroup_getCtx(g)
 * ============================================================================
 */

// CONSTRUCTORS

RadioGroup *RadioGroup_0(void) {
    RadioGroup *g = (RadioGroup*) Memory_alloc(TYPE_RADIOGROUP_SINGLETON, sizeof(RadioGroup));
    if (!g)
        return nullptr;
    Panel *base = Panel_0();
    if (!base) {
        Memory_free(g);
        return nullptr;
    }
    (*g).base = (*base);
    Memory_free(base);
    (*g).options = nullptr;
    (*g).selected = -1;
    (*g).orientation = RADIOGROUP_VERTICAL;
    (*g).onSelect = nullptr;
    (*g).ctx = nullptr;
    return g;
}

RadioGroup *RadioGroup_1(Panel *parent) {
    RadioGroup *g = RadioGroup_0();
    if (g && parent)
        Panel_addContainer(parent, &(*g).base);
    return g;
}

// CORE FUNCTIONS

void RadioGroup_addOption(RadioGroup *g, const char *option) {
    ;;INCOMPLETE // owned-string append lands with the layout pass
    (void)g;
    (void)option;
}

void RadioGroup_clear(RadioGroup *g) {
    ;;INCOMPLETE // owned-string drain lands with the layout pass
    (void)g;
}

// SETTERS

static void markDirty(RadioGroup *g) {
    if (!g)
        return;
    Panel *b = &(*g).base;
    Container_markDirty(&(*b).base);
}

void RadioGroup_setSelected(RadioGroup *g, int32_t index) {
    if (!g)
        return;
    (*g).selected = index;
    markDirty(g);
}

void RadioGroup_setOrientation(RadioGroup *g, int32_t orientation) {
    if (!g)
        return;
    (*g).orientation = (orientation == RADIOGROUP_HORIZONTAL) ? 1 : 0;
    markDirty(g);
}

void RadioGroup_setOnSelect(RadioGroup *g, RadioGroupSelectFn cb) {
    if (!g)
        return;
    (*g).onSelect = cb;
}

void RadioGroup_setCtx(RadioGroup *g, void *ctx) {
    if (!g)
        return;
    (*g).ctx = ctx;
}

void RadioGroup_free(RadioGroup *g) {
    if (!g)
        return;
    List *opts = (*g).options;
    if (opts) {
        size_t n = List_size(opts);
        for (size_t i = 0; i < n; i++) {
            char *item = (char*) (uintptr_t) List_get(opts, i);
            if (item)
                Memory_free(item);
        }
        List_free(opts);
        (*g).options = nullptr;
    }
    Memory_free(g);
}

// GETTERS

int32_t RadioGroup_getSelected(const RadioGroup *g) {
    return g ? (*g).selected : -1;
}

int32_t RadioGroup_getOrientation(const RadioGroup *g) {
    return g ? (*g).orientation : RADIOGROUP_VERTICAL;
}

int32_t RadioGroup_optionCount(const RadioGroup *g) {
    if (!g || !(*g).options)
        return 0;
    List *opts = (*g).options;
    return (int32_t) List_size(opts);
}

const char *RadioGroup_getOption(const RadioGroup *g, int32_t index) {
    if (!g || !(*g).options || index < 0)
        return nullptr;
    List *opts = (*g).options;
    if ((size_t) index >= List_size(opts))
        return nullptr;
    return (const char*) (uintptr_t) List_get(opts, (size_t) index);
}

RadioGroupSelectFn RadioGroup_getOnSelect(const RadioGroup *g) {
    return g ? (*g).onSelect : nullptr;
}

void *RadioGroup_getCtx(const RadioGroup *g) {
    return g ? (*g).ctx : nullptr;
}
