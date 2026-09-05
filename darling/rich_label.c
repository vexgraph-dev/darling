#include "rich_label.h"
#include "vulkan/vk.h"
#include "nio/mem.h"
#include "oop/type.h"
#include "annotation/overview.h"

;;OVERVIEW
/**
 * ============================================================================
 * CLASS: RichLabel (inherits Panel)
 * LEVEL: L2 — Behavior (rich text UI view behavior API)
 * ============================================================================
 * Retained-mode rich-text view: a Panel hosting a RichText layout model with
 * an inherited wrap mode, rendered through the SDF atlas path.
 *
 * STRUCT FIELDS (Mirroring darling/rich_label.h):
 * ----------------------------------------------------------------------------
 *   Panel base;            // Inherited layout, bounds, and hierarchy state
 *   RichText *textModel;   // Owned styled-text layout model (see rich_text.h)
 *   WrapMode wrapMode;     // Line-wrap policy inherited by the layout engine
 *
 * FUNCTION REGISTRY:
 * ----------------------------------------------------------------------------
 * Constructors:
 *   - RichLabel_0(void)
 *   - RichLabel_1(parent)
 *
 * Setters:
 *   - RichLabel_setTextModel(label, model)
 *   - RichLabel_setWrapMode(label, mode)
 * ============================================================================
 */


static void RichLabel_renderFn(Panel *panel, void *renderer, void *cmdBuffer, float x, float y, float w, float h) {
    RichLabel *rl = (RichLabel*) panel;
    (void)renderer;
    
    uint32_t bgColor = Panel_getBackgroundColor(panel);
    if ((bgColor >> 24) > 0) {
        float br = ((bgColor >> 16) & 0xFF) / 255.0f;
        float bg = ((bgColor >> 8) & 0xFF) / 255.0f;
        float bb = (bgColor & 0xFF) / 255.0f;
        float ba = ((bgColor >> 24) & 0xFF) / 255.0f;
        Vk_fillRect(cmdBuffer, w, h, x, y, w, h, br, bg, bb, ba);
    }
    
    RichText *tm = (*rl).textModel;
    if (!tm || !(*tm).quads) return;
    
    for (size_t i = 0; i < (*tm).quadCount; i++) {
        TextQuad *q = &(*tm).quads[i];
        
        float cr = (((*q).color >> 16) & 0xFF) / 255.0f;
        float cg = (((*q).color >> 8) & 0xFF) / 255.0f;
        float cb = ((*q).color & 0xFF) / 255.0f;
        float ca = (((*q).color >> 24) & 0xFF) / 255.0f;
        
        float qx = x + (*q).x;
        float qy = y + (*q).y;
        
        if ((*q).decor != DECOR_NONE || (*q).textureId < 0) {
            // TODO: Pass decor to a specialized shader for Dash/Squiggle.
            // For now, it draws a solid line.
            Vk_fillRect(cmdBuffer, w, h, qx, qy, (*q).w, (*q).h, cr, cg, cb, ca);
        } else if ((*q).isColor) {
            Vk_drawColorGlyph(cmdBuffer, w, h, qx, qy, (*q).w, (*q).h,
                              ca, (*q).textureId, (*q).u0, (*q).v0, (*q).u1, (*q).v1);
        } else {
            Vk_drawSDFText(cmdBuffer, w, h, qx, qy, (*q).w, (*q).h, 
                           cr, cg, cb, ca, (*q).textureId, (*q).bold, 0.5f,
                           (*q).u0, (*q).v0, (*q).u1, (*q).v1);
        }
    }
}

RichLabel *RichLabel_0(void) {
    RichLabel *rl = (RichLabel*) Memory_alloc(TYPE_PANEL_SINGLETON, sizeof(RichLabel));
    if (!rl) return NULL;
    
    Panel *p = Panel_0();
    if (!p) {
        Memory_free(rl);
        return NULL;
    }
    
    (*rl).base = *p;
    Memory_free(p);
    
    (*rl).textModel = NULL;
    (*rl).wrapMode = WRAP_WORD;
    Panel_setRenderHandler(&(*rl).base, RichLabel_renderFn);
    
    return rl;
}

RichLabel *RichLabel_1(Panel *parent) {
    RichLabel *rl = RichLabel_0();
    if (rl && parent) {
        Panel_addContainer(parent, &(*rl).base);
    }
    return rl;
}

void RichLabel_setTextModel(RichLabel *label, RichText *model) {
    if (!label) return;
    (*label).textModel = model;
    Container_markDirty(&(*label).base.base);
}

void RichLabel_setWrapMode(RichLabel *label, WrapMode mode) {
    if (!label) return;
    (*label).wrapMode = mode;
    if ((*label).textModel) {
        RichText_setWrapMode((*label).textModel, mode);
    }
    Container_markDirty(&(*label).base.base);
}
