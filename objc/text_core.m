#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import <CoreText/CoreText.h>
#import <CoreGraphics/CoreGraphics.h>

#include <stdlib.h>
#include <string.h>

#include "text/text_core.h"

#include "annotation/draft.h"
#include "annotation/intention.h"
#include "annotation/overview.h"

;;OVERVIEW
/**
 * ============================================================================
 * MODULE: Text_core (objc/text_core.m)
 * LEVEL: L4 — Self-Management (OS CoreText line raster shim)
 * ============================================================================
 * native line raster seam.
 *
 * FUNCTION REGISTRY:
 * ----------------------------------------------------------------------------
 * Core Functions:
 *   - TextCore_backingScale(void)
 *   - TextCore_rasterLine(utf8, family, pxHeight, argb, outRgba, outW, outH)
 * ============================================================================
 */


;;DRAFT
;;INTENTION("Native CoreText line raster for sharp Label, active-mode backing")

float TextCore_backingScale(void) {
    CGFloat s = [[NSScreen mainScreen] backingScaleFactor];
    if (s > 0.0)
        return (float) s;
    return 1.0f;
}

bool TextCore_rasterLine(const char *utf8, const char *family, float pxHeight, uint32_t argb, uint8_t **outRgba, int *outW, int *outH) {
    if (!utf8 || !outRgba || !outW || !outH)
        return false;
    if (pxHeight <= 0.0f)
        return false;
    (*outRgba) = nullptr;
    (*outW) = 0;
    (*outH) = 0;

    @autoreleasepool {
        NSString *str = [NSString stringWithUTF8String:utf8];
        if (!str)
            return false;
        NSString *fam = family ? [NSString stringWithUTF8String:family] : @"Helvetica";
        CTFontRef font = CTFontCreateWithName((__bridge CFStringRef) fam, pxHeight, NULL);
        if (!font) {
            font = CTFontCreateWithName(CFSTR("Helvetica"), pxHeight, NULL);
            if (!font)
                return false;
        }
        CGGlyph testG[2] = {0};
        UniChar testC[2] = {'a', 'e'};
        if (CTFontGetGlyphsForCharacters(font, testC, testG, 2) && testG[0] == testG[1]) {
            CFRelease(font);
            font = CTFontCreateWithName(CFSTR("Menlo"), pxHeight, NULL);
            if (!font)
                font = CTFontCreateWithName(CFSTR("Helvetica"), pxHeight, NULL);
            if (!font)
                return false;
        }

        uint8_t a = (uint8_t) ((argb >> 24) & 0xFF);
        uint8_t r = (uint8_t) ((argb >> 16) & 0xFF);
        uint8_t g = (uint8_t) ((argb >> 8) & 0xFF);
        uint8_t b = (uint8_t) (argb & 0xFF);

        NSDictionary *attrs = @{
            (id)kCTFontAttributeName: (__bridge id) font,
            (id)kCTForegroundColorAttributeName: (id)[NSColor colorWithCalibratedRed:(r/255.0) green:(g/255.0) blue:(b/255.0) alpha:(a/255.0)].CGColor
        };
        // Multiline: split on \n, one CTLine per row, stacked top to bottom.
        NSArray<NSString *> *rows = [str componentsSeparatedByString:@"\n"];
        if ([rows count] == 0)
            rows = @[str];
        size_t nlines = [rows count];
        if (nlines > 64)
            nlines = 64;
        CTLineRef lines[64];
        CGFloat asc[64];
        CGFloat desc[64];
        CGFloat lead[64];
        double advs[64];
        for (size_t k = 0; k < nlines; k++) {
            NSString *row = [rows objectAtIndex:k];
            NSAttributedString *rattr = [[NSAttributedString alloc] initWithString:row attributes:attrs];
            CTLineRef line = CTLineCreateWithAttributedString((__bridge CFAttributedStringRef) rattr);
            lines[k] = line;
            asc[k] = 0;
            desc[k] = 0;
            lead[k] = 0;
            advs[k] = 0;
            if (line)
                advs[k] = CTLineGetTypographicBounds(line, &asc[k], &desc[k], &lead[k]);
        }
        CFRelease(font);
        double maxAdv = 0;
        double totalH = 0;
        for (size_t k = 0; k < nlines; k++) {
            if (advs[k] > maxAdv)
                maxAdv = advs[k];
            totalH += asc[k] + desc[k] + lead[k];
        }
        int w = (int) ceil(maxAdv) + 2;
        int h = (int) ceil(totalH) + (int) (2 * nlines);
        bool okLines = true;
        for (size_t k = 0; k < nlines; k++) {
            if (!lines[k])
                okLines = false;
        }
        if (!okLines) {
            for (size_t k = 0; k < nlines; k++) {
                if (lines[k])
                    CFRelease(lines[k]);
            }
            return false;
        }
        if (w <= 0 || h <= 0) {
            for (size_t k = 0; k < nlines; k++)
                CFRelease(lines[k]);
            return false;
        }
        if (w > 8192)
            w = 8192;
        if (h > 4096)
            h = 4096;

        size_t rowBytes = (size_t) w * 4;
        uint8_t *buf = (uint8_t*) calloc((size_t) h, rowBytes);
        if (!buf) {
            for (size_t k = 0; k < nlines; k++)
                CFRelease(lines[k]);
            return false;
        }

        CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
        CGContextRef ctx = CGBitmapContextCreate(buf, w, h, 8, rowBytes, cs,
            kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);
        CGColorSpaceRelease(cs);
        if (!ctx) {
            free(buf);
            for (size_t k = 0; k < nlines; k++)
                CFRelease(lines[k]);
            return false;
        }

        CGContextSetShouldAntialias(ctx, true);
        CGContextSetAllowsFontSmoothing(ctx, false);
        CGContextSetShouldSmoothFonts(ctx, false);
        CGContextSetAllowsFontSubpixelPositioning(ctx, true);
        CGContextSetShouldSubpixelPositionFonts(ctx, true);
        CGContextSetAllowsFontSubpixelQuantization(ctx, true);
        CGContextSetShouldSubpixelQuantizeFonts(ctx, true);
        CGContextClearRect(ctx, CGRectMake(0, 0, w, h));
        // Stack lines from the bottom: last row at 1+descent, earlier above.
        // In CGBitmapContext, row 0 in memory is already visual top with upright glyphs.
        {
            double penY = 1.0;
            for (size_t k = nlines; k > 0; k--) {
                size_t idx = k - 1;
                CGContextSetTextPosition(ctx, 1.0, penY + desc[idx]);
                CTLineDraw(lines[idx], ctx);
                penY += asc[idx] + desc[idx] + lead[idx] + 2.0;
            }
        }
        CGContextRelease(ctx);
        for (size_t k = 0; k < nlines; k++)
            CFRelease(lines[k]);

        (*outRgba) = buf;
        (*outW) = w;
        (*outH) = h;
        return true;
    }
}
