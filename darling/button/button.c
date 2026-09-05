#include "darling/button/button.h"

#include "darling/panel/panel.h"
#include "annotation/incomplete.h"
#include "annotation/overview.h"
#include "nio/mem.h"
#include "oop/type.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

;;OVERVIEW
/**
 * ============================================================================
 * CLASS: Button (embeds Panel)
 * LEVEL: L2 — Behavior (pressable button shell)
 * ============================================================================
 * Panel shell for a pressable button with an owned label, font styling,
 * per-state colors, and a press callback. Hit-testing and press dispatch
 * land in a later pass.
 *
 * STRUCT FIELDS (Mirroring darling/button/button.h):
 * ----------------------------------------------------------------------------
 *   Panel base;                        // Inherited layout/tree/background state
 *   char *label;                       // Owned label copy; nullptr = empty
 *   Font *font;                        // Borrowed font handle; nullptr = default
 *   float fontSize;                    // Label height in points
 *   uint32_t textColor;                // Packed 0xAARRGGBB label color
 *   uint32_t bg;                       // Packed 0xAARRGGBB idle fill
 *   uint32_t bgHover;                  // Packed 0xAARRGGBB hover fill
 *   uint32_t bgPressed;                // Packed 0xAARRGGBB pressed fill
 *   uint32_t borderColor;              // Packed 0xAARRGGBB border color
 *   float radius;                      // Corner radius in points
 *   float borderWidth;                 // Border width in points
 *   bool disabled;                     // True = non-interactive
 *   bool hovered;                      // True = pointer currently over
 *   bool pressed;                      // True = currently held down
 *   void (*onPress)(void *ctx);        // Press callback; nullptr = none
 *   void *ctx;                         // Callback context
 *
 * FUNCTION REGISTRY:
 * ----------------------------------------------------------------------------
 * Constructors:
 *   - Button_0(void)
 *   - Button_1(label)
 *   - Button_2(parent, label)
 *
 * Core Functions:
 *   - Button_press(b)
 *   - Button_free(b)
 *
 * Setters:
 *   - Button_setLabel(b, label)
 *   - Button_setFont(b, font)
 *   - Button_setFontSize(b, size)
 *   - Button_setTextColor(b, color)
 *   - Button_setBackground(b, color)
 *   - Button_setBackgroundHover(b, color)
 *   - Button_setBackgroundPressed(b, color)
 *   - Button_setBorderColor(b, color)
 *   - Button_setRadius(b, radius)
 *   - Button_setBorderWidth(b, width)
 *   - Button_setDisabled(b, disabled)
 *   - Button_setHovered(b, hovered)
 *   - Button_setPressed(b, pressed)
 *   - Button_setOnPress(b, fn, ctx)
 *
 * Getters:
 *   - Button_getLabel(b)
 *   - Button_getFont(b)
 *   - Button_getFontSize(b)
 *   - Button_getTextColor(b)
 *   - Button_getBackground(b)
 *   - Button_getBackgroundHover(b)
 *   - Button_getBackgroundPressed(b)
 *   - Button_getBorderColor(b)
 *   - Button_getRadius(b)
 *   - Button_getBorderWidth(b)
 *   - Button_isDisabled(b)
 *   - Button_isHovered(b)
 *   - Button_isPressed(b)
 *   - Button_getOnPress(b)
 *   - Button_getPressContext(b)
 * ============================================================================
 */

// CONSTRUCTORS
// ============================================================================

Button *Button_0(void) {
    Button *b = (Button*) Memory_alloc(TYPE_BUTTON_SINGLETON, sizeof(Button));
    if (!b)
        return nullptr;
    Panel *p = Panel_0();
    if (!p) {
        Memory_free(b);
        return nullptr;
    }
    (*b).base = (*p);
    Memory_free(p);
    (*b).label = nullptr;
    (*b).font = nullptr;
    (*b).fontSize = 12.0f;
    (*b).textColor = 0xFFFFFFFFu;
    (*b).bg = 0xFF3A3A3Au;
    (*b).bgHover = 0xFF4A4A4Au;
    (*b).bgPressed = 0xFF2A2A2Au;
    (*b).borderColor = 0xFF888888u;
    (*b).radius = 4.0f;
    (*b).borderWidth = 1.0f;
    (*b).disabled = false;
    (*b).hovered = false;
    (*b).pressed = false;
    (*b).onPress = nullptr;
    (*b).ctx = nullptr;
    return b;
}

Button *Button_1(const char *label) {
    Button *b = Button_0();
    if (b)
        Button_setLabel(b, label);
    return b;
}

Button *Button_2(Panel *parent, const char *label) {
    Button *b = Button_1(label);
    if (b && parent) {
        Panel *p = &(*b).base;
        Panel_addContainer(parent, p);
    }
    return b;
}

// CORE FUNCTIONS
// ============================================================================

void Button_press(Button *b) {
    ;;INCOMPLETE // hit-test plus press dispatch deferred
    (void) b;
}

void Button_free(Button *b) {
    if (!b)
        return;
    if ((*b).label)
        Memory_free((*b).label);
    (*b).label = nullptr;
    Memory_free(b);
}

// SETTERS
// ============================================================================

static void markDirty(Button *b) {
    if (!b)
        return;
    Panel *p = &(*b).base;
    Container *c = &(*p).base;
    Container_markDirty(c);
}

void Button_setLabel(Button *b, const char *label) {
    if (!b)
        return;
    if ((*b).label)
        Memory_free((*b).label);
    (*b).label = nullptr;
    if (label) {
        size_t len = strlen(label) + 1;
        (*b).label = (char*) Memory_alloc(TYPE_ARRAY, len);
        if ((*b).label)
            strcpy((*b).label, label);
    }
    markDirty(b);
}

void Button_setFont(Button *b, Font *font) {
    if (!b)
        return;
    (*b).font = font;
    markDirty(b);
}

void Button_setFontSize(Button *b, float size) {
    if (!b)
        return;
    (*b).fontSize = size;
    markDirty(b);
}

void Button_setTextColor(Button *b, uint32_t color) {
    if (!b)
        return;
    (*b).textColor = color;
    markDirty(b);
}

void Button_setBackground(Button *b, uint32_t color) {
    if (!b)
        return;
    (*b).bg = color;
    markDirty(b);
}

void Button_setBackgroundHover(Button *b, uint32_t color) {
    if (!b)
        return;
    (*b).bgHover = color;
    markDirty(b);
}

void Button_setBackgroundPressed(Button *b, uint32_t color) {
    if (!b)
        return;
    (*b).bgPressed = color;
    markDirty(b);
}

void Button_setBorderColor(Button *b, uint32_t color) {
    if (!b)
        return;
    (*b).borderColor = color;
    markDirty(b);
}

void Button_setRadius(Button *b, float radius) {
    if (!b)
        return;
    (*b).radius = radius;
    markDirty(b);
}

void Button_setBorderWidth(Button *b, float width) {
    if (!b)
        return;
    (*b).borderWidth = width;
    markDirty(b);
}

void Button_setDisabled(Button *b, bool disabled) {
    if (!b)
        return;
    (*b).disabled = disabled;
    markDirty(b);
}

void Button_setHovered(Button *b, bool hovered) {
    if (!b)
        return;
    (*b).hovered = hovered;
    markDirty(b);
}

void Button_setPressed(Button *b, bool pressed) {
    if (!b)
        return;
    (*b).pressed = pressed;
    markDirty(b);
}

void Button_setOnPress(Button *b, void (*fn)(void *ctx), void *ctx) {
    if (!b)
        return;
    (*b).onPress = fn;
    (*b).ctx = ctx;
}

// GETTERS
// ============================================================================

const char *Button_getLabel(const Button *b) {
    return b ? (*b).label : nullptr;
}

Font *Button_getFont(const Button *b) {
    return b ? (*b).font : nullptr;
}

float Button_getFontSize(const Button *b) {
    return b ? (*b).fontSize : 0.0f;
}

uint32_t Button_getTextColor(const Button *b) {
    return b ? (*b).textColor : 0u;
}

uint32_t Button_getBackground(const Button *b) {
    return b ? (*b).bg : 0u;
}

uint32_t Button_getBackgroundHover(const Button *b) {
    return b ? (*b).bgHover : 0u;
}

uint32_t Button_getBackgroundPressed(const Button *b) {
    return b ? (*b).bgPressed : 0u;
}

uint32_t Button_getBorderColor(const Button *b) {
    return b ? (*b).borderColor : 0u;
}

float Button_getRadius(const Button *b) {
    return b ? (*b).radius : 0.0f;
}

float Button_getBorderWidth(const Button *b) {
    return b ? (*b).borderWidth : 0.0f;
}

bool Button_isDisabled(const Button *b) {
    return b ? (*b).disabled : false;
}

bool Button_isHovered(const Button *b) {
    return b ? (*b).hovered : false;
}

bool Button_isPressed(const Button *b) {
    return b ? (*b).pressed : false;
}

void (*Button_getOnPress(const Button *b))(void *ctx) {
    return b ? (*b).onPress : nullptr;
}

void *Button_getPressContext(const Button *b) {
    return b ? (*b).ctx : nullptr;
}
