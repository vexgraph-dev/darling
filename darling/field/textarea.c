#include "darling/field/textarea.h"

#include <string.h>

#include "annotation/incomplete.h"
#include "event/keyevent.h"
#include "event/pointer.h"
#include "input/key.h"
#include "nio/mem.h"
#include "oop/type.h"
#include "annotation/overview.h"

;;OVERVIEW
/**
 * ============================================================================
 * CLASS: Textarea (inherits Panel, LEVEL L2 Behavior)
 * ============================================================================
 * Multi-line text area shell: Panel layout plus an owned text buffer with
 * visible-line count, word-wrap mode, and vertical scroll offset.
 *
 * LIVE EDITING (Pkg 4): byte-wise caret-addressed insert/erase, enter for
 * `\n`, backspace (joins across newlines), left/right by one char, up/down
 * across line boundaries by scanning `\n` (column clamped to the target
 * line length). Every edit fires onChange(ctx) — a notification only, state
 * is pulled via the getters. Caret-follow scroll clamps scrollY so the
 * caret line stays in [scrollY, scrollY + visibleLines - 1]; lineHeight is
 * 1 unit because no font metrics exist yet (documented assumption), so
 * scrollY is in lines.
 *
 * STRUCT FIELDS (Mirroring darling/field/textarea.h):
 * ----------------------------------------------------------------------------
 *   Panel base;            // Inherited layout, bounds, and hierarchy state
 *   char *text;            // Owned UTF-8 buffer
 *   int32_t visibleLines;  // Viewport height in lines
 *   int32_t wrap;          // Wrap mode (0 = off, 1 = word)
 *   float scrollY;         // Vertical scroll offset in lines (lineHeight = 1)
 *   Font *font;            // Optional SDF font descriptor (borrowed)
 *   int32_t cursor;        // Caret offset into text
 *   bool focused;          // True once PTR_DOWN lands (dispatch owns the rest)
 *   Textarea_ChangeFn onChange; // Edit callback; nullptr = none
 *   void *ctx;             // Callback context (borrowed)
 *
 * FUNCTION REGISTRY:
 * ----------------------------------------------------------------------------
 * Constructors:
 *   - Textarea()                     : Textarea_0()
 *   - Textarea(parent, visibleLines) : Textarea_2(parent, visibleLines)
 *
 * Core Functions:
 *   - Textarea_scrollTo(ta, y)
 *   - Textarea_handlePointer(self, kind, localX, localY)
 *   - Textarea_handleKey(self, ev)
 *
 * Setters:
 *   - Textarea_goTo(self, index)
 *   - Textarea_setText(ta, text)
 *   - Textarea_setVisibleLines(ta, lines)
 *   - Textarea_setWrap(ta, wrap)
 *   - Textarea_setScrollY(ta, y)
 *   - Textarea_setFont(ta, font)
 *   - Textarea_setCursor(self, cursor)
 *   - Textarea_setOnChange(self, fn)
 *   - Textarea_setCtx(self, ctx)
 *   - Textarea_free(ta)
 *
 * Getters:
 *   - Textarea_getText(ta)
 *   - Textarea_getVisibleLines(ta)
 *   - Textarea_getWrap(ta)
 *   - Textarea_getScrollY(ta)
 *   - Textarea_getFont(ta)
 *   - Textarea_getCursor(self)
 *   - Textarea_isFocused(self)
 *   - Textarea_getOnChange(self)
 *   - Textarea_getCtx(self)
 * ============================================================================
 */

// ============================================================================
// INTERNAL HELPERS
// ============================================================================

static int32_t clampCursor(size_t len, int32_t cursor) {
    int32_t n = len > (size_t)INT32_MAX ? INT32_MAX : (int32_t)len;
    if (cursor < 0)
        return 0;
    if (cursor > n)
        return n;
    return cursor;
}

static size_t textLen(const Textarea *self) {
    char *cur = self ? (*self).text : nullptr;
    return cur ? strlen(cur) : 0;
}

// ============================================================================
// CONSTRUCTORS
// ============================================================================

#define TEXTAREA_DEFAULT_LINES 4

Textarea *Textarea_0(void) {
    Textarea *ta = (Textarea*) Memory_alloc(TYPE_TEXTAREA_SINGLETON, sizeof(Textarea));
    if (!ta)
        return nullptr;
    Panel *bp = Panel_0();
    if (!bp) {
        Memory_free(ta);
        return nullptr;
    }
    (*ta).base = (*bp);
    Memory_free(bp);
    (*ta).text = nullptr;
    (*ta).visibleLines = TEXTAREA_DEFAULT_LINES;
    (*ta).wrap = 1;
    (*ta).scrollY = 0.0f;
    (*ta).font = nullptr;
    (*ta).cursor = 0;
    (*ta).focused = false;
    (*ta).onChange = nullptr;
    (*ta).ctx = nullptr;
    return ta;
}

Textarea *Textarea_2(Panel *parent, int32_t visibleLines) {
    Textarea *ta = Textarea_0();
    if (!ta)
        return nullptr;
    (*ta).visibleLines = visibleLines;
    if (parent) {
        Panel *bp = &(*ta).base;
        Panel_addContainer(parent, bp);
    }
    return ta;
}

// ============================================================================
// CORE FUNCTIONS
// ============================================================================

void Textarea_scrollTo(Textarea *ta, float y) {
    ;;INCOMPLETE // clamped scroll lands with the scroll walker
    (void)ta;
    (void)y;
}

// ============================================================================
// SETTERS
// ============================================================================

static void markDirty(Textarea *ta) {
    if (!ta)
        return;
    Panel *bp = &(*ta).base;
    Container_markDirty(&(*bp).base);
}

void Textarea_setText(Textarea *ta, const char *text) {
    if (!ta)
        return;
    char *old = (*ta).text;
    if (old) {
        Memory_free(old);
        (*ta).text = nullptr;
    }
    if (text) {
        size_t len = strlen(text) + 1;
        char *buf = (char*) Memory_alloc(TYPE_ARRAY, len);
        if (buf)
            memcpy(buf, text, len);
        (*ta).text = buf;
    }
    (*ta).cursor = clampCursor(textLen(ta), (*ta).cursor);
    markDirty(ta);
}

void Textarea_setVisibleLines(Textarea *ta, int32_t lines) {
    if (!ta)
        return;
    (*ta).visibleLines = lines;
    markDirty(ta);
}

void Textarea_setWrap(Textarea *ta, int32_t wrap) {
    if (!ta)
        return;
    (*ta).wrap = wrap;
    markDirty(ta);
}

void Textarea_setScrollY(Textarea *ta, float y) {
    if (!ta)
        return;
    (*ta).scrollY = y;
    markDirty(ta);
}

void Textarea_setFont(Textarea *ta, Font *font) {
    if (!ta)
        return;
    (*ta).font = font;
    markDirty(ta);
}

void Textarea_goTo(Textarea *self, int32_t index) {
    if (!self)
        return;
    (*self).cursor = clampCursor(textLen(self), index);
    markDirty(self);
}

void Textarea_setCursor(Textarea *self, int32_t cursor) {
    Textarea_goTo(self, cursor);
}

void Textarea_setOnChange(Textarea *self, Textarea_ChangeFn fn) {
    if (!self)
        return;
    (*self).onChange = fn;
}

void Textarea_setCtx(Textarea *self, void *ctx) {
    if (!self)
        return;
    (*self).ctx = ctx;
}

void Textarea_free(Textarea *ta) {
    if (!ta)
        return;
    char *text = (*ta).text;
    if (text)
        Memory_free(text);
    (*ta).text = nullptr;
    (*ta).font = nullptr;
    (*ta).onChange = nullptr;
    (*ta).ctx = nullptr;
    Memory_free(ta);
}

// ============================================================================
// LIVE EDITING (Pkg 4)
// ============================================================================

static void fireChange(Textarea *self) {
    if (!self)
        return;
    Textarea_ChangeFn fn = (*self).onChange;
    if (fn)
        fn((*self).ctx);
}

// Caret line = count of '\n' before the cursor.
static int32_t caretLine(const Textarea *self) {
    if (!self)
        return 0;
    char *cur = (*self).text;
    int32_t at = (*self).cursor;
    int32_t line = 0;
    for (int32_t i = 0; i < at; i++) {
        if (!cur || !cur[i])
            break;
        if (cur[i] == '\n')
            line++;
    }
    return line;
}

// Caret-follow scroll: clamp scrollY so the caret line stays in
// [scrollY, scrollY + visibleLines - 1]. Line height is 1 unit — no font
// metrics exist yet, so scrollY is counted in lines (documented assumption).
static void followCaret(Textarea *self) {
    if (!self)
        return;
    int32_t line = caretLine(self);
    int32_t vis = (*self).visibleLines;
    if (vis < 1)
        vis = 1;
    float top = (*self).scrollY;
    if ((float)line < top)
        top = (float)line;
    if ((float)line > top + (float)(vis - 1))
        top = (float)line - (float)(vis - 1);
    if (top < 0.0f)
        top = 0.0f;
    if (top != (*self).scrollY) {
        (*self).scrollY = top;
        markDirty(self);
    }
}

// Line 0 starts at 0; each '\n' at i starts a line at i + 1.
static int32_t lineStart(const char *text, int32_t line) {
    int32_t start = 0;
    int32_t cur = 0;
    if (!text)
        return 0;
    while (cur < line && text[start]) {
        if (text[start] == '\n')
            cur++;
        start++;
    }
    return start;
}

static int32_t lineLen(const char *text, int32_t start) {
    int32_t n = 0;
    if (!text)
        return 0;
    while (text[start + n] && text[start + n] != '\n')
        n++;
    return n;
}

static int32_t lineCount(const char *text) {
    int32_t n = 1;
    if (!text)
        return 1;
    for (int32_t i = 0; text[i]; i++)
        if (text[i] == '\n')
            n++;
    return n;
}

static void insertAt(Textarea *self, char c) {
    if (!self)
        return;
    char *cur = (*self).text;
    size_t len = cur ? strlen(cur) : 0;
    int32_t at = clampCursor(len, (*self).cursor);
    char *buf = (char*) Memory_alloc(TYPE_ARRAY, len + 2);
    if (!buf)
        return;
    if (cur && (size_t)at > 0)
        memcpy(buf, cur, (size_t)at);
    buf[at] = c;
    if (cur)
        memcpy(buf + at + 1, cur + at, len - (size_t)at + 1);
    else
        buf[at + 1] = '\0';
    if (cur)
        Memory_free(cur);
    (*self).text = buf;
    (*self).cursor = at + 1;
    followCaret(self);
    markDirty(self);
    fireChange(self);
}

static void eraseAt(Textarea *self) {
    if (!self)
        return;
    char *cur = (*self).text;
    if (!cur)
        return;
    size_t len = strlen(cur);
    int32_t at = clampCursor(len, (*self).cursor);
    if (at <= 0)
        return;
    memmove(cur + at - 1, cur + at, len - (size_t)at + 1);
    (*self).cursor = at - 1;
    followCaret(self);
    markDirty(self);
    fireChange(self);
}

static void moveVertical(Textarea *self, int32_t dir) {
    if (!self)
        return;
    char *cur = (*self).text;
    int32_t line = caretLine(self);
    int32_t col = clampCursor(textLen(self), (*self).cursor) - lineStart(cur, line);
    int32_t next = line + dir;
    if (next < 0)
        return;
    if (next >= lineCount(cur))
        return;
    int32_t start = lineStart(cur, next);
    int32_t n = lineLen(cur, start);
    if (col > n)
        col = n;
    Textarea_goTo(self, start + col);
    followCaret(self);
}

void Textarea_handlePointer(Textarea *self, int kind, float localX, float localY) {
    if (!self)
        return;
    (void)localX;
    (void)localY;
    if (kind == PTR_DOWN)
        (*self).focused = true;
}

void Textarea_handleKey(Textarea *self, const UIKeyEvent *ev) {
    if (!self)
        return;
    if (!ev)
        return;
    if (!UIKeyEvent_isPressed(ev))
        return;
    int32_t code = UIKeyEvent_getKeyCode(ev);
    int32_t ch = UIKeyEvent_getCh(ev);
    if (code == KEY_BACKSPACE) {
        eraseAt(self);
        return;
    }
    if (code == KEY_LEFT) {
        Textarea_goTo(self, (*self).cursor - 1);
        followCaret(self);
        return;
    }
    if (code == KEY_RIGHT) {
        Textarea_goTo(self, (*self).cursor + 1);
        followCaret(self);
        return;
    }
    if (code == KEY_UP) {
        moveVertical(self, -1);
        return;
    }
    if (code == KEY_DOWN) {
        moveVertical(self, 1);
        return;
    }
    if (code == KEY_ENTER || ch == '\n' || ch == '\r') {
        insertAt(self, '\n');
        return;
    }
    if (ch >= 32 && ch <= 126)
        insertAt(self, (char)ch);
}

// ============================================================================
// GETTERS
// ============================================================================

const char *Textarea_getText(const Textarea *ta) {
    return ta ? (*ta).text : nullptr;
}

int32_t Textarea_getVisibleLines(const Textarea *ta) {
    return ta ? (*ta).visibleLines : 0;
}

int32_t Textarea_getWrap(const Textarea *ta) {
    return ta ? (*ta).wrap : 0;
}

float Textarea_getScrollY(const Textarea *ta) {
    return ta ? (*ta).scrollY : 0.0f;
}

Font *Textarea_getFont(const Textarea *ta) {
    return ta ? (*ta).font : nullptr;
}

int32_t Textarea_getCursor(const Textarea *self) {
    return self ? (*self).cursor : 0;
}

bool Textarea_isFocused(const Textarea *self) {
    return self && (*self).focused;
}

Textarea_ChangeFn Textarea_getOnChange(const Textarea *self) {
    return self ? (*self).onChange : nullptr;
}

void *Textarea_getCtx(const Textarea *self) {
    return self ? (*self).ctx : nullptr;
}
