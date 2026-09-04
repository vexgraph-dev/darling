#include "annotation/overview.h"
#include "darling/picture.h"
#include "nio/mem.h"
#include "oop/type.h"

;;OVERVIEW
/**
 * ============================================================================
 * CLASS: Picture (inherits Panel -> Container)
 * ============================================================================
 * Retained-mode off-heap picture node that hosts an Image asset with optional
 * UV cropping, dimension overrides, and hierarchical layout anchoring.
 *
 * STRUCT FIELDS (Mirroring darling/picture.h):
 * ----------------------------------------------------------------------------
 *   Panel base;            // Base UI panel state (bounds, anchors, background)
 *   void *image;           // Pointer to raw Image asset descriptor
 *   float imageSizeW;      // Explicit pixel display width (-1 = auto)
 *   float imageSizeH;      // Explicit pixel display height (-1 = auto)
 *   float cropX1, cropY1;  // Normalized UV top-left crop bounds
 *   float cropX2, cropY2;  // Normalized UV bottom-right crop bounds
 *   bool hasImageSize;     // Explicit dimension override active flag
 *   bool hasCrop;          // Custom UV crop rect active flag
 *
 * FUNCTION REGISTRY:
 * ----------------------------------------------------------------------------
 * Constructors:
 *   - Picture()                               : Picture_0()
 *   - Picture(image)                          : Picture_1(image)
 *
 * Core Functions:
 *   - (Leverages base Panel render handler; overridden via Panel_setRenderHandler)
 *
 * Setters:
 *   - Picture_setImage(p, image)
 *   - Picture_setImageSize(p, w, h)
 *   - Picture_setCrop(p, x1, y1, x2, y2)
 *
 * Getters:
 *   - Picture_getImage(p)
 *   - Picture_getImageSize(p, outW, outH)
 *   - Picture_hasImageSize(p)
 *   - Picture_getCrop(p, outX1, outY1, outX2, outY2)
 *   - Picture_hasCrop(p)
 * ============================================================================
 */

// ============================================================================
// CONSTRUCTORS
// ============================================================================

Picture *Picture_0() {
    Picture *p = (Picture*) Memory_alloc(TYPE_PICTURE_SINGLETON, sizeof(Picture));
    if (!p) return nullptr;

    // Allocate the base Panel structure correctly, mimicking Scene allocation.
    // Panel_0() gives us a fully initialized UI panel.
    Panel *basePanel = Panel_0();
    if (!basePanel) {
        Memory_free(p);
        return nullptr;
    }
    
    // Copy initialized state into our base struct, then free the heap-allocated one
    (*p).base = (*basePanel);
    Memory_free(basePanel);

    // Initialize Picture-specific fields (default matching legacy initDefaults)
    (*p).image = nullptr;
    (*p).imageSizeW = 0.0f;
    (*p).imageSizeH = 0.0f;
    (*p).cropX1 = 0.0f;
    (*p).cropY1 = 0.0f;
    (*p).cropX2 = 0.0f;
    (*p).cropY2 = 0.0f;
    (*p).hasImageSize = false;
    (*p).hasCrop = false;

    return p;
}

Picture *Picture_1(void *image) {
    Picture *p = Picture_0();
    if (p) {
        (*p).image = image;
    }
    return p;
}

// ============================================================================
// CORE FUNCTIONS
// ============================================================================

// (Custom drawing hooks register dynamically via Panel_setRenderHandler)

// ============================================================================
// SETTERS
// ============================================================================

void Picture_setImage(Picture *p, void *image) {
    if (p) {
        (*p).image = image;
        Container_markDirty(&(*p).base.base); // Panel's base is Container
    }
}

void Picture_setImageSize(Picture *p, float w, float h) {
    if (p) {
        (*p).imageSizeW = w;
        (*p).imageSizeH = h;
        (*p).hasImageSize = true;
        Container_markDirty(&(*p).base.base);
    }
}

void Picture_setCrop(Picture *p, float x1, float y1, float x2, float y2) {
    if (p) {
        (*p).cropX1 = x1;
        (*p).cropY1 = y1;
        (*p).cropX2 = x2;
        (*p).cropY2 = y2;
        (*p).hasCrop = true;
        Container_markDirty(&(*p).base.base);
    }
}

// ============================================================================
// GETTERS
// ============================================================================

void *Picture_getImage(const Picture *p) {
    return p ? (*p).image : nullptr;
}

void Picture_getImageSize(const Picture *p, float *outW, float *outH) {
    if (outW) (*outW) = p ? (*p).imageSizeW : 0.0f;
    if (outH) (*outH) = p ? (*p).imageSizeH : 0.0f;
}

bool Picture_hasImageSize(const Picture *p) {
    return p ? (*p).hasImageSize : false;
}

void Picture_getCrop(const Picture *p, float *outX1, float *outY1, float *outX2, float *outY2) {
    if (outX1) *outX1 = p ? (*p).cropX1 : 0.0f;
    if (outY1) *outY1 = p ? (*p).cropY1 : 0.0f;
    if (outX2) *outX2 = p ? (*p).cropX2 : 0.0f;
    if (outY2) *outY2 = p ? (*p).cropY2 : 0.0f;
}

bool Picture_hasCrop(const Picture *p) {
    return p ? (*p).hasCrop : false;
}
