#ifndef DARLING_LABEL_H
#define DARLING_LABEL_H

#include "panel.h"
#include "font/font.h"
#include <stdint.h>
#include "c23/constructor.h"

// A lightweight View component for simple, single-styled text.
// Sharp path: one native CoreText raster per line (single textured quad).
// RichLabel keeps the SDF atlas path for mask/fill effects.
typedef struct Label {
    Panel base;
    char *text;
    Font *font;
    char *fontFamily;
    float fontSize;
    uint32_t textColor;
    float smoothness;
    int32_t rasterTex;
    int rasterW;
    int rasterH;
    float rasterBacking;
    bool rasterDirty;
} Label;

Label *Label_0(void);
Label *Label_1(const char *text);
Label *Label_2(Panel *parent, const char *text);
Label *Label_1_parent(Panel *parent);

#define Label(...) CONSTRUCTOR_DISPATCH(Label, __VA_ARGS__)

void Label_setText(Label *label, const char *text);
void Label_setFont(Label *label, Font *font);
void Label_setFontFamily(Label *label, const char *family);
void Label_setFontSize(Label *label, float size);
void Label_setTextColor(Label *label, uint32_t color);
void Label_setSmoothness(Label *label, float smoothness);
void Label_free(Label *label);
void Label_setLocation(Label *label, float x, float y);
void Label_setSize(Label *label, float w, float h);
void Label_setBackgroundColor(Label *label, uint32_t color);

// Symmetric Getters (Java-library standard)
const char *Label_getText(const Label *label);
Font *Label_getFont(const Label *label);
const char *Label_getFontFamily(const Label *label);
float Label_getFontSize(const Label *label);
uint32_t Label_getTextColor(const Label *label);
float Label_getSmoothness(const Label *label);
int32_t Label_getRasterTexture(const Label *label);
void Label_getRasterSize(const Label *label, int *outW, int *outH);
float Label_getRasterBacking(const Label *label);
bool Label_isRasterDirty(const Label *label);

#endif // DARLING_LABEL_H
