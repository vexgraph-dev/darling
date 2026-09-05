#ifndef DARLING_FIELD_TEXTAREA_H
#define DARLING_FIELD_TEXTAREA_H

#include <stdbool.h>
#include <stdint.h>

#include "c23/constructor.h"
#include "darling/panel/panel.h"
#include "font/font.h"

// Multi-line text area: Panel layout plus an owned buffer with scroll state.
typedef struct Textarea {
    Panel base;
    char *text;
    int32_t visibleLines;
    int32_t wrap;
    float scrollY;
    Font *font;
} Textarea;

Textarea *Textarea_0(void);
Textarea *Textarea_2(Panel *parent, int32_t visibleLines);

#define Textarea(...) CONSTRUCTOR_DISPATCH(Textarea, __VA_ARGS__)

// Core scroll (stub: clamping lands with the scroll walker).
void Textarea_scrollTo(Textarea *ta, float y);

void Textarea_free(Textarea *ta);

void Textarea_setText(Textarea *ta, const char *text);
void Textarea_setVisibleLines(Textarea *ta, int32_t lines);
void Textarea_setWrap(Textarea *ta, int32_t wrap);
void Textarea_setScrollY(Textarea *ta, float y);
void Textarea_setFont(Textarea *ta, Font *font);

const char *Textarea_getText(const Textarea *ta);
int32_t Textarea_getVisibleLines(const Textarea *ta);
int32_t Textarea_getWrap(const Textarea *ta);
float Textarea_getScrollY(const Textarea *ta);
Font *Textarea_getFont(const Textarea *ta);

#endif
