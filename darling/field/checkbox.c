#include "darling/field/checkbox.h"

#include "annotation/overview.h"
#include "nio/mem.h"
#include "oop/type.h"

;;OVERVIEW
/**
 * ============================================================================
 * CLASS: Checkbox (embeds Panel)
 * LEVEL: L2 — Behavior (boolean toggle behavior API)
 * ============================================================================
 * Boolean toggle with an indeterminate tri-state flag and a change callback.
 *
 * STRUCT FIELDS (Mirroring darling/field/checkbox.h):
 * ----------------------------------------------------------------------------
 *   Panel base;                  // Inherited layout, bounds, hierarchy state
 *   bool checked;                // Current on/off state
 *   bool indeterminate;          // Tri-state dash flag for mixed children
 *   uint32_t box;                // Box outline color, packed 0xAARRGGBB
 *   uint32_t check;              // Checkmark fill color, packed 0xAARRGGBB
 *   void (*onChange)(void *ctx); // Change callback; nullptr means none
 *   void *ctx;                   // Callback context pointer
 *
 * FUNCTION REGISTRY:
 * ----------------------------------------------------------------------------
 * Constructors:
 *   - Checkbox_0(void)
 *   - Checkbox_1(parent)
 *
 * Core Functions:
 *   - Checkbox_toggle(c)
 *
 * Setters:
 *   - Checkbox_setChecked(c, checked)
 *   - Checkbox_setIndeterminate(c, value)
 *   - Checkbox_setBox(c, color)
 *   - Checkbox_setCheck(c, color)
 *   - Checkbox_setOnChange(c, cb)
 *   - Checkbox_setCtx(c, ctx)
 *
 * Getters:
 *   - Checkbox_isChecked(c)
 *   - Checkbox_isIndeterminate(c)
 *   - Checkbox_getBox(c)
 *   - Checkbox_getCheck(c)
 *   - Checkbox_getOnChange(c)
 *   - Checkbox_getCtx(c)
 * ============================================================================
 */

// CONSTRUCTORS

Checkbox *Checkbox_0(void) {
    Checkbox *c = (Checkbox*) Memory_alloc(TYPE_CHECKBOX_SINGLETON, sizeof(Checkbox));
    if (!c)
        return nullptr;
    Panel *base = Panel_0();
    if (!base) {
        Memory_free(c);
        return nullptr;
    }
    (*c).base = (*base);
    Memory_free(base);
    (*c).checked = false;
    (*c).indeterminate = false;
    (*c).box = 0xFF888888u;
    (*c).check = 0xFF222222u;
    (*c).onChange = nullptr;
    (*c).ctx = nullptr;
    return c;
}

Checkbox *Checkbox_1(Panel *parent) {
    Checkbox *c = Checkbox_0();
    if (c && parent)
        Panel_addContainer(parent, &(*c).base);
    return c;
}

// CORE FUNCTIONS

void Checkbox_toggle(Checkbox *c) {
    if (c)
        Checkbox_setChecked(c, !Checkbox_isChecked(c));
}

// SETTERS

static void markDirty(Checkbox *c) {
    if (!c)
        return;
    Panel *b = &(*c).base;
    Container_markDirty(&(*b).base);
}

void Checkbox_setChecked(Checkbox *c, bool checked) {
    if (!c)
        return;
    (*c).checked = checked;
    markDirty(c);
}

void Checkbox_setIndeterminate(Checkbox *c, bool value) {
    if (!c)
        return;
    (*c).indeterminate = value;
    markDirty(c);
}

void Checkbox_setBox(Checkbox *c, uint32_t color) {
    if (!c)
        return;
    (*c).box = color;
    markDirty(c);
}

void Checkbox_setCheck(Checkbox *c, uint32_t color) {
    if (!c)
        return;
    (*c).check = color;
    markDirty(c);
}

void Checkbox_setOnChange(Checkbox *c, CheckboxChangeFn cb) {
    if (!c)
        return;
    (*c).onChange = cb;
}

void Checkbox_setCtx(Checkbox *c, void *ctx) {
    if (!c)
        return;
    (*c).ctx = ctx;
}

// GETTERS

bool Checkbox_isChecked(const Checkbox *c) {
    return c ? (*c).checked : false;
}

bool Checkbox_isIndeterminate(const Checkbox *c) {
    return c ? (*c).indeterminate : false;
}

uint32_t Checkbox_getBox(const Checkbox *c) {
    return c ? (*c).box : 0;
}

uint32_t Checkbox_getCheck(const Checkbox *c) {
    return c ? (*c).check : 0;
}

CheckboxChangeFn Checkbox_getOnChange(const Checkbox *c) {
    return c ? (*c).onChange : nullptr;
}

void *Checkbox_getCtx(const Checkbox *c) {
    return c ? (*c).ctx : nullptr;
}
