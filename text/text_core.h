#ifndef TEXT_CORE_H
#define TEXT_CORE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// text/text_core.h — native line raster seam.
//
// Label (sharp native path) asks the platform to rasterize one line of UTF-8
// into RGBA8 at native physical pixels. RichLabel stays on the SDF atlas path
// for mask/fill effects.
//
// Backing comes from the active pixel mode (currentWidth / pointWidth), not
// the panel max and not a hardcoded 2.0.

// Active backing scale: NSScreen backingScaleFactor (Retina points to pixels).
// For active-mode currentWidth/pointWidth, combine with DisplayInfo on top.
float TextCore_backingScale(void);

// Rasterize one UTF-8 line. pxHeight is native pixels (points * backing).
// Returns malloc'd RGBA8 (caller frees with free), or nullptr on failure.
// outW/outH receive native pixel dimensions. Last param is dest-last.
bool TextCore_rasterLine(const char *utf8, const char *family, float pxHeight, uint32_t argb, uint8_t **outRgba, int *outW, int *outH);

#endif
