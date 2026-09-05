#include "annotation/overview.h"
#include "label.h"
#include "vulkan/vk.h"
#include "nio/mem.h"
#include "oop/type.h"
#include "text/text_core.h"
#include "vulkan/texture/texture.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

;;OVERVIEW
/**
 * ============================================================================
 * CLASS: Label (inherits Panel -> Container)
 * LEVEL: L2 — Behavior (UI text view behavior API)
 * ============================================================================
 * Lightweight retained-mode UI text view for sharp, single-styled typography.
 * Implements a dual-path rendering strategy:
 *   1. Sharp Path : Native CoreText line rasterization into a textured quad.
 *   2. SDF Path   : Multi-pass signed distance field fallback rendering.
 *
 * STRUCT FIELDS (Mirroring darling/label/label.h):
 * ----------------------------------------------------------------------------
 *   Panel base;              // Inherited layout, bounds, and hierarchy state
 *   char *text;              // UTF-8 string payload
 *   Font *font;              // Optional SDF font atlas descriptor
 *   char *fontFamily;        // CoreText typeface family name (e.g. "Helvetica")
 *   float fontSize;          // Font height in points
 *   uint32_t textColor;      // Packed 0xAARRGGBB color value
 *   float smoothness;        // SDF edge anti-aliasing sharpness factor
 *   int32_t rasterTex;       // GPU texture ID of CoreText cached raster quad
 *   int rasterW;             // Pixel width of CoreText raster
 *   int rasterH;             // Pixel height of CoreText raster
 *   float rasterBacking;     // Retina scale factor at rasterization time
 *   bool rasterDirty;        // True if string or font changed and needs re-raster
 *
 * FUNCTION REGISTRY:
 * ----------------------------------------------------------------------------
 * Constructors:
 *   - Label()                                : Label_0()
 *   - Label(text)                            : Label_1(text)
 *   - Label(parent, text)                    : Label_2(parent, text)
 *   - Label_1_parent(parent)
 *
 * Core Functions:
 *   - Label_renderFn(panel, rend, cmd, x, y, w, h) : Draw handler
 *
 * Setters:
 *   - Label_setText(label, text)
 *   - Label_setFont(label, font)
 *   - Label_setFontFamily(label, family)
 *   - Label_setFontSize(label, size)
 *   - Label_setTextColor(label, color)
 *   - Label_setSmoothness(label, smoothness)
 *   - Label_setLocation(label, x, y)
 *   - Label_setSize(label, w, h)
 *   - Label_setBackgroundColor(label, color)
 *   - Label_free(label)
 *
 * Getters:
 *   - Label_getText(const label)
 *   - Label_getFont(const label)
 *   - Label_getFontFamily(const label)
 *   - Label_getFontSize(const label)
 *   - Label_getTextColor(const label)
 *   - Label_getSmoothness(const label)
 *   - Label_getRasterTexture(const label)
 *   - Label_getRasterSize(const label, outW, outH)
 *   - Label_getRasterBacking(const label)
 *   - Label_isRasterDirty(const label)
 * ============================================================================
 */

// ============================================================================
// CORE FUNCTIONS
// ============================================================================

static void markRasterDirty(Label *lbl) {
    if (!lbl)
        return;
    (*lbl).rasterDirty = true;
}

static bool ensureRaster(Label *lbl) {
    if (!lbl)
        return false;
    if (!(*lbl).rasterDirty)
        return (*lbl).rasterTex >= 0;
    (*lbl).rasterDirty = false;
    if (!(*lbl).text || (*lbl).text[0] == '\0' || (*lbl).fontSize <= 0.0f) {
        (*lbl).rasterTex = -1;
        return false;
    }
    float backing = TextCore_backingScale();
    if (backing <= 0.0f)
        backing = 1.0f;
    float pxH = (*lbl).fontSize * backing;
    if (pxH <= 0.0f)
        return false;
    const char *family = (*lbl).fontFamily ? (*lbl).fontFamily : "Helvetica";
    // Multiline Label: full text goes to CoreText, \n stacks rows.
    // SDF fallback below still walks \n itself.
    const char *srcText = (*lbl).text;
    uint8_t *rgba = nullptr;
    int w = 0;
    int h = 0;
    if (!TextCore_rasterLine(srcText, family, pxH, (*lbl).textColor, &rgba, &w, &h))
        return false;
    if (!rgba || w <= 0 || h <= 0)
        return false;
    int32_t tex = Texture_loadRaw(rgba, (uint32_t) w, (uint32_t) h);
    free(rgba);
    if (tex < 0)
        return false;
    (*lbl).rasterTex = tex;
    (*lbl).rasterW = w;
    (*lbl).rasterH = h;
    (*lbl).rasterBacking = backing;
    return true;
}

static void drawSdfFallback(Panel *panel, void *cmdBuffer, float x, float y, float w, float h) {
    Label *lbl = (Label*) panel;
    uint32_t bgColor = Panel_getBackgroundColor(panel);
    if ((bgColor >> 24) > 0) {
        float br = ((bgColor >> 16) & 0xFF) / 255.0f;
        float bg = ((bgColor >> 8) & 0xFF) / 255.0f;
        float bb = (bgColor & 0xFF) / 255.0f;
        float ba = ((bgColor >> 24) & 0xFF) / 255.0f;
        Vk_fillRect(cmdBuffer, w, h, x, y, w, h, br, bg, bb, ba);
    }
    if (!(*lbl).text || !(*lbl).font || (*lbl).fontSize <= 0)
        return;
    float cr = (((*lbl).textColor >> 16) & 0xFF) / 255.0f;
    float cg = (((*lbl).textColor >> 8) & 0xFF) / 255.0f;
    float cb = ((*lbl).textColor & 0xFF) / 255.0f;
    float ca = (((*lbl).textColor >> 24) & 0xFF) / 255.0f;
    float ascent = 0;
    float descent = 0;
    float lineGap = 0;
    Font_getVMetrics((*lbl).font, &ascent, &descent, &lineGap);
    float scale = Font_getScaleForPixelHeight((*lbl).font, (*lbl).fontSize);
    float lineHeight = (ascent - descent + lineGap) * scale;
    if (lineHeight <= 0.0f)
        lineHeight = (*lbl).fontSize * 1.2f;

    GlyphMetrics spaceGm = {0};
    float spaceAdvance = (*lbl).fontSize * 0.3f;
    if (Font_getGlyph((*lbl).font, (uint32_t) ' ', (*lbl).fontSize, &spaceGm) && spaceGm.advance > 0.0f)
        spaceAdvance = spaceGm.advance;
    float tabWidth = 4.0f * spaceAdvance;

    float cx = x;
    float cy = y + (ascent * scale);
    int32_t page0Tex = Font_getTextureId((*lbl).font);
    uint32_t prevChar = 0;
    int len = (int) strlen((*lbl).text);
    for (int i = 0; i < len; ) {
        uint32_t codepoint = 0;
        unsigned char c0 = (unsigned char) (*lbl).text[i];
        int charLen = 1;
        if (c0 < 0x80) {
            codepoint = c0;
        } else if ((c0 & 0xE0) == 0xC0) {
            if (i + 1 < len)
                codepoint = ((c0 & 0x1F) << 6) | ((*lbl).text[i + 1] & 0x3F);
            charLen = 2;
        } else if ((c0 & 0xF0) == 0xE0) {
            if (i + 2 < len)
                codepoint = ((c0 & 0x0F) << 12) | (((*lbl).text[i + 1] & 0x3F) << 6) | ((*lbl).text[i + 2] & 0x3F);
            charLen = 3;
        } else if ((c0 & 0xF8) == 0xF0) {
            if (i + 3 < len)
                codepoint = ((c0 & 0x07) << 18) | (((*lbl).text[i + 1] & 0x3F) << 12) | (((*lbl).text[i + 2] & 0x3F) << 6) | ((*lbl).text[i + 3] & 0x3F);
            charLen = 4;
        }
        if (codepoint == '\r') {
            prevChar = 0;
            i += charLen;
            continue;
        }
        if (codepoint == '\n') {
            cx = x;
            cy += lineHeight;
            prevChar = 0;
            i += charLen;
            continue;
        }
        if (codepoint == '\t') {
            if (tabWidth > 0.0f) {
                float relX = cx - x;
                cx = x + (floorf(relX / tabWidth) + 1.0f) * tabWidth;
            } else {
                cx += 4.0f * spaceAdvance;
            }
            prevChar = 0;
            i += charLen;
            continue;
        }
        GlyphMetrics gm = {0};
        if (Font_getGlyph((*lbl).font, codepoint, (*lbl).fontSize, &gm)) {
            if (prevChar != 0)
                cx += Font_getKerning((*lbl).font, prevChar, codepoint, (*lbl).fontSize);
            prevChar = codepoint;
            if (gm.width > 0.0f && gm.height > 0.0f) {
                float qx = cx + gm.xOffset;
                float qy = cy + gm.yOffset;
                // Multi-page atlas: each glyph draws from its own page.
                // Color glyphs (runtime emoji) take the raw-RGBA branch.
                int32_t texId = Font_pageTextureId((*lbl).font, (size_t)gm.page);
                if (texId < 0)
                    texId = page0Tex;
                if (gm.color)
                    Vk_drawColorGlyph(cmdBuffer, w, h, qx, qy, gm.width, gm.height, ca, texId, gm.u0, gm.v0, gm.u1, gm.v1);
                else
                    Vk_drawSDFText(cmdBuffer, w, h, qx, qy, gm.width, gm.height, cr, cg, cb, ca, texId, 0.0f, (*lbl).smoothness, gm.u0, gm.v0, gm.u1, gm.v1);
            }
            cx += gm.advance;
        }
        i += charLen;
    }
}

static void Label_renderFn(Panel *panel, void *renderer, void *cmdBuffer, float x, float y, float w, float h) {
    Label *lbl = (Label*) panel;
    (void) renderer;
    uint32_t bgColor = Panel_getBackgroundColor(panel);
    if ((bgColor >> 24) > 0) {
        float br = ((bgColor >> 16) & 0xFF) / 255.0f;
        float bgg = ((bgColor >> 8) & 0xFF) / 255.0f;
        float bb = (bgColor & 0xFF) / 255.0f;
        float ba = ((bgColor >> 24) & 0xFF) / 255.0f;
        Vk_fillRect(cmdBuffer, w, h, x, y, w, h, br, bgg, bb, ba);
    }
    if (!lbl || !(*lbl).text || (*lbl).text[0] == '\0' || (*lbl).fontSize <= 0.0f)
        return;
    // Sharp path: one native raster quad, top-left anchored in panel.
    // macOS panels are bottom-up (AppKit): panel origin is bottom-left,
    // so pin quad top to y + h - qh. Falls back to SDF per-glyph below.
    if (ensureRaster(lbl) && (*lbl).rasterTex >= 0 && (*lbl).rasterW > 0 && (*lbl).rasterH > 0) {
        float backing = (*lbl).rasterBacking;
        if (backing <= 0.0f)
            backing = 1.0f;
        float qw = (float) (*lbl).rasterW;
        float qh = (float) (*lbl).rasterH;
        Panel *basePanel = &(*lbl).base;
        Container *container = &(*basePanel).base;
        if (w <= (*container).w * 1.25f && backing > 1.0f) {
            qw /= backing;
            qh /= backing;
        }
        float qx = x;
        float qy = y + h - qh;
        Vk_drawTexture(cmdBuffer, w, h, qx, qy, qw, qh, 1.0f, 1.0f, 1.0f, 1.0f,
            (*lbl).rasterTex, PICTURE_MODE_FIT, (float) (*lbl).rasterW, (float) (*lbl).rasterH);
        return;
    }
    drawSdfFallback(panel, cmdBuffer, x, y, w, h);
}

// ============================================================================
// CONSTRUCTORS
// ============================================================================

Label *Label_0(void) {
    Label *lbl = (Label*) Memory_alloc(TYPE_PANEL_SINGLETON, sizeof(Label));
    if (!lbl)
        return NULL;
    Panel *p = Panel_0();
    if (!p) {
        Memory_free(lbl);
        return NULL;
    }
    (*lbl).base = *p;
    Memory_free(p);
    (*lbl).text = NULL;
    (*lbl).font = NULL;
    (*lbl).fontFamily = nullptr;
    (*lbl).fontSize = 12.0f;
    (*lbl).textColor = 0xFFFFFFFF;
    (*lbl).smoothness = 0.5f;
    (*lbl).rasterTex = -1;
    (*lbl).rasterW = 0;
    (*lbl).rasterH = 0;
    (*lbl).rasterBacking = 1.0f;
    (*lbl).rasterDirty = true;
    {
        const char *defFamily = "Helvetica";
        size_t defLen = strlen(defFamily) + 1;
        (*lbl).fontFamily = (char*) Memory_alloc(TYPE_ARRAY, defLen);
        if ((*lbl).fontFamily)
            strcpy((*lbl).fontFamily, defFamily);
    }
    Panel_setRenderHandler(&(*lbl).base, Label_renderFn);
    return lbl;
}

Label *Label_1(const char *text) {
    Label *lbl = Label_0();
    if (lbl)
        Label_setText(lbl, text);
    return lbl;
}

Label *Label_1_parent(Panel *parent) {
    Label *lbl = Label_0();
    if (lbl && parent)
        Panel_addContainer(parent, &(*lbl).base);
    return lbl;
}

Label *Label_2(Panel *parent, const char *text) {
    Label *lbl = Label_1(text);
    if (lbl && parent)
        Panel_addContainer(parent, &(*lbl).base);
    return lbl;
}

// ============================================================================
// SETTERS
// ============================================================================

static void markDirty(Label *lbl) {
    if (!lbl)
        return;
    Panel *p = &(*lbl).base;
    Container_markDirty(&(*p).base);
}

void Label_setText(Label *label, const char *text) {
    if (!label)
        return;
    if ((*label).text)
        Memory_free((*label).text);
    if (text) {
        (*label).text = (char*) Memory_alloc(TYPE_ARRAY, strlen(text) + 1);
        if ((*label).text)
            strcpy((*label).text, text);
    } else {
        (*label).text = NULL;
    }
    markRasterDirty(label);
    markDirty(label);
}

void Label_setFont(Label *label, Font *font) {
    if (!label)
        return;
    (*label).font = font;
    markRasterDirty(label);
    markDirty(label);
}

void Label_setFontFamily(Label *label, const char *family) {
    if (!label)
        return;
    if ((*label).fontFamily)
        Memory_free((*label).fontFamily);
    (*label).fontFamily = nullptr;
    if (family) {
        size_t len = strlen(family) + 1;
        (*label).fontFamily = (char*) Memory_alloc(TYPE_ARRAY, len);
        if ((*label).fontFamily)
            strcpy((*label).fontFamily, family);
    }
    markRasterDirty(label);
    markDirty(label);
}

void Label_setFontSize(Label *label, float size) {
    if (!label)
        return;
    (*label).fontSize = size;
    markRasterDirty(label);
    markDirty(label);
}

void Label_setTextColor(Label *label, uint32_t color) {
    if (!label)
        return;
    (*label).textColor = color;
    markRasterDirty(label);
    markDirty(label);
}

void Label_setSmoothness(Label *label, float smoothness) {
    if (!label)
        return;
    if (smoothness < 0.0f)
        smoothness = 0.0f;
    if (smoothness > 1.0f)
        smoothness = 1.0f;
    (*label).smoothness = smoothness;
    markDirty(label);
}

void Label_setLocation(Label *label, float x, float y) {
    if (!label)
        return;
    Panel_setLocation(&(*label).base, x, y);
}

void Label_setSize(Label *label, float w, float h) {
    if (!label)
        return;
    Panel_setSize(&(*label).base, w, h);
}

void Label_setBackgroundColor(Label *label, uint32_t color) {
    if (!label)
        return;
    Panel_setBackgroundColor(&(*label).base, color);
}

void Label_free(Label *label) {
    if (!label)
        return;
    if ((*label).text)
        Memory_free((*label).text);
    if ((*label).fontFamily)
        Memory_free((*label).fontFamily);
    (*label).text = nullptr;
    (*label).fontFamily = nullptr;
    (*label).rasterTex = -1;
    Memory_free(label);
}

// ============================================================================
// GETTERS
// ============================================================================

const char *Label_getText(const Label *label) {
    return label ? (*label).text : nullptr;
}

Font *Label_getFont(const Label *label) {
    return label ? (*label).font : nullptr;
}

const char *Label_getFontFamily(const Label *label) {
    return label ? (*label).fontFamily : nullptr;
}

float Label_getFontSize(const Label *label) {
    return label ? (*label).fontSize : 0.0f;
}

uint32_t Label_getTextColor(const Label *label) {
    return label ? (*label).textColor : 0;
}

float Label_getSmoothness(const Label *label) {
    return label ? (*label).smoothness : 0.0f;
}

int32_t Label_getRasterTexture(const Label *label) {
    return label ? (*label).rasterTex : -1;
}

void Label_getRasterSize(const Label *label, int *outW, int *outH) {
    if (outW) (*outW) = label ? (*label).rasterW : 0;
    if (outH) (*outH) = label ? (*label).rasterH : 0;
}

float Label_getRasterBacking(const Label *label) {
    return label ? (*label).rasterBacking : 1.0f;
}

bool Label_isRasterDirty(const Label *label) {
    return label ? (*label).rasterDirty : false;
}
