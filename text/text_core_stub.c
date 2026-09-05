#include "text/text_core.h"

#include <stdlib.h>

#include "annotation/draft.h"
#include "annotation/intention.h"
#include "annotation/overview.h"

;;OVERVIEW
/**
 * ============================================================================
 * MODULE: Text_core_stub (text/text_core_stub.c)
 * LEVEL: L2 — Behavior (portable text fallback behavior API)
 * ============================================================================
 * non-Apple fallback. No native raster here;
 *
 * STRUCT FIELDS: none — procedural (stateless TextCore seam fallback).
 *
 * FUNCTION REGISTRY:
 * ----------------------------------------------------------------------------
 * Core Functions:
 *   - TextCore_backingScale(void)
 *   - TextCore_rasterLine(utf8, family, pxHeight, argb, outRgba, outW, outH)
 * ============================================================================
 */


// text/text_core_stub.c — non-Apple fallback. No native raster here;
// Label falls back to the SDF atlas path.
;;DRAFT
;;INTENTION("Stub native raster off Apple, SDF fallback owns text")

float TextCore_backingScale(void) {
    return 1.0f;
}

bool TextCore_rasterLine(const char *utf8, const char *family, float pxHeight, uint32_t argb, uint8_t **outRgba, int *outW, int *outH) {
    if (!utf8 || !family)
        return false;
    if (pxHeight <= 0.0f)
        return false;
    if (!outRgba || !outW || !outH)
        return false;
    if (!argb)
        return false;
    (*outRgba) = nullptr;
    (*outW) = 0;
    (*outH) = 0;
    return false;
}
