#include "darling/field/input.h"

#include <math.h>
#include <string.h>

#include "annotation/incomplete.h"
#include "darling/anim/anim.h"
#include "event/keyevent.h"
#include "event/pointer.h"
#include "input/key.h"
#include "nio/mem.h"
#include "oop/type.h"
#include "annotation/overview.h"

;;OVERVIEW
/**
 * ============================================================================
 * CLASS: Input (inherits Panel, LEVEL L2 Behavior)
 * ============================================================================
 * Single-line text input shell: Panel layout plus an owned text buffer
 * bounded by cap, with change/submit callback slots for later wiring.
 *
 * THE CARET (its own part — field->caret->verb):
 * ----------------------------------------------------------------------------
 * The caret is a VIEW over typing state, never pierced directly. It owns
 * mode (BLINK terminal / SOLID always-on / GLIDE Word-style eased slide),
 * color, blink half-period, the blink clock/phase, and the painted x which
 * eases toward the owner-measured target in GLIDE mode (ANIM_EASE_OUT over
 * 80ms via Anim_eval — the animation system, reused, not reinvented).
 * Position resolution (cursor index -> x) lands with the caret walker; the
 * owner places the target with caret_setTarget after measuring. Typing
 * restarts the blink phase shown. Tick on Thread 0 next to layout.
 *
 * STRUCT FIELDS (Mirroring darling/field/input.h — same part banners):
 * ----------------------------------------------------------------------------
 *   --- Input core (owner fields) ---
 *   Panel base;              // Inherited layout, bounds, and hierarchy state
 *   char *text;              // Owned UTF-8 buffer (bounded by cap)
 *   size_t cap;              // Max stored chars excluding NUL
 *   char *placeholder;       // Owned hint string shown when empty
 *   bool password;           // Mask glyphs at render time
 *   bool readonly;           // Reject edits, still selectable
 *   bool focused;            // Focus-request flag (DOWN sets, dispatch consumes later)
 *   int32_t cursor;          // Caret offset into text
 *   Font *font;              // Optional SDF font descriptor (borrowed)
 *   --- Caret part (views only) ---
 *   int caretMode;           // BLINK/SOLID/GLIDE (default BLINK)
 *   uint32_t caretColor;     // Packed 0xAARRGGBB (default white)
 *   float caretBlinkPeriod;  // Half-cycle seconds (default 0.53)
 *   double caretClock;       // Blink timer (tick advances)
 *   bool caretShown;         // Current blink phase (view)
 *   float caretX;            // Painted x (glides to target)
 *   float caretTargetX;      // Owner-measured x (view target)
 *   Panel *caretView;        // Borrowed visual (null = thin rect)
 *   float caretOpacity;      // User opacity 0..1 (× blink phase)
 *   --- Input core callbacks (owner fields, continued) ---
 *   Input_ChangeFn onChange; // Edit callback; nullptr = none
 *   Input_SubmitFn onSubmit; // Commit callback; nullptr = none
 *   void *ctx;               // Callback context (borrowed)
 *   Input_MeasureFn measurer;// Index->x hook (null until the walker lands)
 *   void *measureCtx;        // Measure context (borrowed)
 *
 * FUNCTION REGISTRY:
 * ----------------------------------------------------------------------------
 * Constructors:
 *   - Input()                : Input_0()
 *   - Input(parent, cap)     : Input_2(parent, cap)
 *
 * Core Functions:
 *   - Input_insertChar(inp, c)
 *   - Input_eraseChar(inp)
 *   - Input_handlePointer(self, kind, localX, localY)
 *   - Input_handleKey(self, ev)
 *
 * Setters:
 *   - Input_goTo(inp, index)
 *   - Input_setText(inp, text)
 *   - Input_setCap(inp, cap)
 *   - Input_setPlaceholder(inp, placeholder)
 *   - Input_setPassword(inp, password)
 *   - Input_setReadonly(inp, readonly)
 *   - Input_setFocused(inp, focused)
 *   - Input_setCursor(inp, cursor)
 *   - Input_setFont(inp, font)
 *   - Input_setOnChange(inp, fn)
 *   - Input_setOnSubmit(inp, fn)
 *   - Input_setCtx(inp, ctx)
 *   - Input_setMeasurer(inp, fn, ctx)
 *   - Input_free(inp)
 *
 * Caret part:
 *   - Input_caret_setMode(inp, mode)
 *   - Input_caret_setColor(inp, color)
 *   - Input_caret_setBlinkPeriod(inp, seconds)
 *   - Input_caret_setTarget(inp, x)
 *   - Input_caret_setView(inp, view)
 *   - Input_caret_setOpacity(inp, opacity)
 *   - Input_caret_placeView(inp, view, centerY)
 *   - Input_caret_tick(inp, dt)
 *
 * Getters:
 *   - Input_getText(inp)
 *   - Input_getCap(inp)
 *   - Input_getPlaceholder(inp)
 *   - Input_isPassword(inp)
 *   - Input_isReadonly(inp)
 *   - Input_isFocused(inp)
 *   - Input_getCursor(inp)
 *   - Input_getFont(inp)
 *   - Input_getOnChange(inp)
 *   - Input_getOnSubmit(inp)
 *   - Input_getCtx(inp)
 *   - Input_getMeasurer(inp)
 *   - Input_getMeasureContext(inp)
 *   - Input_caret_getMode(inp)
 *   - Input_caret_getColor(inp)
 *   - Input_caret_getBlinkPeriod(inp)
 *   - Input_caret_getTarget(inp)
 *   - Input_caret_getX(inp)
 *   - Input_caret_isShown(inp)
 *   - Input_caret_getView(inp)
 *   - Input_caret_getOpacity(inp)
 *   - Input_caret_getEffectiveOpacity(inp)
 * ============================================================================
 */

// ============================================================================
// CONSTRUCTORS
// ============================================================================

#define INPUT_DEFAULT_CAP 256

Input *Input_0(void) {
    Input *inp = (Input*) Memory_alloc(TYPE_INPUT_SINGLETON, sizeof(Input));
    if (!inp)
        return nullptr;
    Panel *bp = Panel_0();
    if (!bp) {
        Memory_free(inp);
        return nullptr;
    }
    (*inp).base = (*bp);
    Memory_free(bp);
    (*inp).text = nullptr;
    (*inp).cap = INPUT_DEFAULT_CAP;
    (*inp).placeholder = nullptr;
    (*inp).password = false;
    (*inp).readonly = false;
    (*inp).focused = false;
    (*inp).cursor = 0;
    (*inp).font = nullptr;
    (*inp).caretMode = INPUT_CARET_BLINK;
    (*inp).caretColor = 0xFFFFFFFFu;
    (*inp).caretBlinkPeriod = INPUT_CARET_DEFAULT_PERIOD;
    (*inp).caretClock = 0.0;
    (*inp).caretShown = true;
    (*inp).caretX = 0.0f;
    (*inp).caretTargetX = 0.0f;
    (*inp).caretView = nullptr;
    (*inp).caretOpacity = 1.0f;
    (*inp).onChange = nullptr;
    (*inp).onSubmit = nullptr;
    (*inp).ctx = nullptr;
    (*inp).measurer = nullptr;
    (*inp).measureCtx = nullptr;
    return inp;
}

Input *Input_2(Panel *parent, size_t cap) {
    Input *inp = Input_0();
    if (!inp)
        return nullptr;
    (*inp).cap = cap;
    if (parent) {
        Panel *bp = &(*inp).base;
        Panel_addContainer(parent, bp);
    }
    return inp;
}

// ============================================================================
// CORE FUNCTIONS
// ============================================================================

static int32_t clampCursor(size_t len, int32_t cursor);
static void markDirty(Input *inp);

void Input_insertChar(Input *inp, char c) {
    if (!inp) return;
    if ((*inp).readonly) return; // reject edits, still selectable
    char *cur = (*inp).text;
    size_t len = cur ? strlen(cur) : 0;
    if (len >= (*inp).cap) return; // cap-truncate like setText: byte does not fit
    int32_t at = clampCursor(len, (*inp).cursor);
    size_t nlen = len + 1;
    char *buf = (char*) Memory_alloc(TYPE_ARRAY, nlen + 1);
    if (!buf) return;
    if (cur && at > 0) memcpy(buf, cur, (size_t)at);
    buf[at] = c;
    if (cur && (size_t)at < len) memcpy(buf + at + 1, cur + at, len - (size_t)at);
    buf[nlen] = '\0';
    if (cur) Memory_free(cur);
    (*inp).text = buf;
    (*inp).cursor = at + 1;
    if ((*inp).measurer)
        Input_caret_setTarget(inp, (*inp).measurer((*inp).measureCtx, (*inp).cursor));
    (*inp).caretClock = 0.0; // typing restarts the blink phase shown
    (*inp).caretShown = true;
    markDirty(inp);
    Input_ChangeFn fn = (*inp).onChange;
    void *ctx = (*inp).ctx;
    if (fn) fn(ctx);
}

void Input_eraseChar(Input *inp) {
    if (!inp) return;
    if ((*inp).readonly) return; // reject edits, still selectable
    char *cur = (*inp).text;
    size_t len = cur ? strlen(cur) : 0;
    int32_t at = clampCursor(len, (*inp).cursor);
    if (at <= 0) return; // nothing before the caret: no change, no fire
    size_t nlen = len - 1;
    char *buf = (char*) Memory_alloc(TYPE_ARRAY, nlen + 1);
    if (!buf) return;
    if (at > 1) memcpy(buf, cur, (size_t)(at - 1));
    if ((size_t)at < len) memcpy(buf + at - 1, cur + at, len - (size_t)at);
    buf[nlen] = '\0';
    Memory_free(cur);
    (*inp).text = buf;
    (*inp).cursor = at - 1;
    if ((*inp).measurer)
        Input_caret_setTarget(inp, (*inp).measurer((*inp).measureCtx, (*inp).cursor));
    (*inp).caretClock = 0.0; // typing restarts the blink phase shown
    (*inp).caretShown = true;
    markDirty(inp);
    Input_ChangeFn fn = (*inp).onChange;
    void *ctx = (*inp).ctx;
    if (fn) fn(ctx);
}

// DOWN requests focus and places the caret at the click (best-effort index
// via the measurer hook, else the end); all other kinds are minimal no-ops.
void Input_handlePointer(Input *self, int kind, float localX, float localY) {
    if (!self) return;
    if (kind != PTR_DOWN) return;
    (void)localY; // single-line: the x run places the caret
    (*self).focused = true;
    char *cur = (*self).text;
    size_t len = cur ? strlen(cur) : 0;
    if ((*self).measurer) {
        int32_t best = 0;
        float bestD = fabsf((*self).measurer((*self).measureCtx, 0) - localX);
        for (int32_t i = 1; (size_t)i <= len; i++) {
            float d = fabsf((*self).measurer((*self).measureCtx, i) - localX);
            if (d < bestD) {
                bestD = d;
                best = i;
            }
        }
        Input_goTo(self, best);
        return;
    }
    int32_t end = len > (size_t)INT32_MAX ? INT32_MAX : (int32_t)len;
    Input_goTo(self, end);
}

// Pressed keys only: printable codepoints insert, backspace erases,
// left/right step the caret (goTo clamps), enter submits. Readonly rejects
// the edits (insert/erase gate themselves); caret moves + submit still run.
void Input_handleKey(Input *self, const UIKeyEvent *ev) {
    if (!self) return;
    if (!ev) return;
    if (!UIKeyEvent_isPressed(ev)) return;
    int32_t code = UIKeyEvent_getKeyCode(ev);
    int32_t ch = UIKeyEvent_getCh(ev);
    if (code == KEY_BACKSPACE || ch == 8) {
        Input_eraseChar(self);
        return;
    }
    if (code == KEY_ENTER || ch == '\r' || ch == '\n') {
        Input_SubmitFn fn = (*self).onSubmit;
        void *ctx = (*self).ctx;
        if (fn) fn(ctx);
        return;
    }
    if (code == KEY_LEFT) {
        Input_goTo(self, (*self).cursor - 1);
        return;
    }
    if (code == KEY_RIGHT) {
        Input_goTo(self, (*self).cursor + 1);
        return;
    }
    if (ch >= 32) Input_insertChar(self, (char)ch);
}

// ============================================================================
// SETTERS
// ============================================================================

static void markDirty(Input *inp) {
    if (!inp)
        return;
    Panel *bp = &(*inp).base;
    Container_markDirty(&(*bp).base);
}

static int32_t clampCursor(size_t len, int32_t cursor) {
    int32_t n = len > (size_t)INT32_MAX ? INT32_MAX : (int32_t)len;
    if (cursor < 0)
        return 0;
    if (cursor > n)
        return n;
    return cursor;
}

void Input_setText(Input *inp, const char *text) {
    if (!inp)
        return;
    char *old = (*inp).text;
    if (old) {
        Memory_free(old);
        (*inp).text = nullptr;
    }
    if (text) {
        size_t len = strlen(text);
        size_t cap = (*inp).cap;
        if (len > cap)
            len = cap;
        char *buf = (char*) Memory_alloc(TYPE_ARRAY, len + 1);
        if (buf) {
            memcpy(buf, text, len);
            buf[len] = '\0';
        }
        (*inp).text = buf;
    }
    char *cur = (*inp).text;
    size_t curLen = cur ? strlen(cur) : 0;
    (*inp).cursor = clampCursor(curLen, (*inp).cursor);
    if ((*inp).measurer)
        Input_caret_setTarget(inp, (*inp).measurer((*inp).measureCtx, (*inp).cursor));
    (*inp).caretClock = 0.0; // typing restarts the blink phase shown
    (*inp).caretShown = true;
    markDirty(inp);
}

void Input_setCap(Input *inp, size_t cap) {
    if (!inp)
        return;
    (*inp).cap = cap;
    char *cur = (*inp).text;
    if (cur && strlen(cur) > cap)
        Input_setText(inp, cur);
    else
        markDirty(inp);
}

void Input_setPlaceholder(Input *inp, const char *placeholder) {
    if (!inp)
        return;
    char *old = (*inp).placeholder;
    if (old) {
        Memory_free(old);
        (*inp).placeholder = nullptr;
    }
    if (placeholder) {
        size_t len = strlen(placeholder) + 1;
        char *buf = (char*) Memory_alloc(TYPE_ARRAY, len);
        if (buf)
            memcpy(buf, placeholder, len);
        (*inp).placeholder = buf;
    }
    markDirty(inp);
}

void Input_setPassword(Input *inp, bool password) {
    if (!inp)
        return;
    (*inp).password = password;
    markDirty(inp);
}

void Input_setReadonly(Input *inp, bool readonly) {
    if (!inp)
        return;
    (*inp).readonly = readonly;
    markDirty(inp);
}

void Input_setFocused(Input *inp, bool focused) {
    if (!inp)
        return;
    (*inp).focused = focused;
    markDirty(inp);
}

void Input_setCursor(Input *inp, int32_t cursor) {
    Input_goTo(inp, cursor); // moving to an index IS going to it
}

// The Word-inspired goTo: every index move re-measures the caret target
// and either blits (BLINK/SOLID) or glides (GLIDE) to it.
void Input_goTo(Input *inp, int32_t index) {
    if (!inp)
        return;
    char *cur = (*inp).text;
    size_t len = cur ? strlen(cur) : 0;
    (*inp).cursor = clampCursor(len, index);
    if ((*inp).measurer)
        Input_caret_setTarget(inp, (*inp).measurer((*inp).measureCtx, (*inp).cursor));
    (*inp).caretClock = 0.0; // arriving restarts the blink phase shown
    (*inp).caretShown = true;
    markDirty(inp);
}

void Input_setFont(Input *inp, Font *font) {
    if (!inp)
        return;
    (*inp).font = font;
    markDirty(inp);
}

void Input_setOnChange(Input *inp, Input_ChangeFn fn) {
    if (!inp)
        return;
    (*inp).onChange = fn;
}

void Input_setOnSubmit(Input *inp, Input_SubmitFn fn) {
    if (!inp)
        return;
    (*inp).onSubmit = fn;
}

void Input_setCtx(Input *inp, void *ctx) {
    if (!inp)
        return;
    (*inp).ctx = ctx;
}

void Input_setMeasurer(Input *inp, Input_MeasureFn fn, void *ctx) {
    if (!inp)
        return;
    (*inp).measurer = fn;
    (*inp).measureCtx = ctx;
}

// ============================================================================
// CARET PART
// ============================================================================

void Input_caret_setMode(Input *inp, int mode) {
    if (!inp)
        return;
    if (mode != INPUT_CARET_BLINK && mode != INPUT_CARET_SOLID && mode != INPUT_CARET_GLIDE)
        return;
    (*inp).caretMode = mode;
    if (mode != INPUT_CARET_BLINK) {
        (*inp).caretShown = true; // SOLID/GLIDE never blink away
        if (mode == INPUT_CARET_SOLID)
            (*inp).caretX = (*inp).caretTargetX;
    }
    markDirty(inp);
}

void Input_caret_setColor(Input *inp, uint32_t color) {
    if (!inp)
        return;
    (*inp).caretColor = color;
    markDirty(inp);
}

void Input_caret_setBlinkPeriod(Input *inp, float seconds) {
    if (!inp || seconds <= 0.0f)
        return;
    (*inp).caretBlinkPeriod = seconds;
    markDirty(inp);
}

void Input_caret_setTarget(Input *inp, float x) {
    if (!inp)
        return;
    (*inp).caretTargetX = x;
    if ((*inp).caretMode != INPUT_CARET_GLIDE)
        (*inp).caretX = x;
    markDirty(inp);
}

void Input_caret_setView(Input *inp, Panel *view) {
    if (!inp)
        return;
    // Borrowed view, detach-only: never freed, never reparented here.
    // Null restores the default thin rect painted by the pump.
    (*inp).caretView = view;
    markDirty(inp);
}

void Input_caret_setOpacity(Input *inp, float opacity) {
    if (!inp)
        return;
    if (opacity < 0.0f)
        opacity = 0.0f;
    if (opacity > 1.0f)
        opacity = 1.0f;
    (*inp).caretOpacity = opacity;
    markDirty(inp);
}

void Input_caret_placeView(Input *inp, Panel *view, float centerY) {
    if (!inp || !view)
        return;
    // Centered, of course: the view's middle lands on (caretX, centerY).
    Container *c = &(*view).base;
    float w = Container_getWidth(c);
    float h = Container_getHeight(c);
    Container_setLocation(c, (*inp).caretX - w * 0.5f, centerY - h * 0.5f);
}

void Input_caret_tick(Input *inp, double dt) {
    if (!inp || dt <= 0.0)
        return;
    if ((*inp).caretMode == INPUT_CARET_BLINK) {
        (*inp).caretClock += dt;
        float period = (*inp).caretBlinkPeriod;
        if (period <= 0.0f)
            period = INPUT_CARET_DEFAULT_PERIOD;
        float phase = fmodf((float)(*inp).caretClock, period * 2.0f);
        bool shown = phase < period; // float-exact: == period hides
        if (shown != (*inp).caretShown) {
            (*inp).caretShown = shown; // dirty ONLY on flip: rest is silence
            markDirty(inp);
        }
    } else if ((*inp).caretMode == INPUT_CARET_GLIDE) {
        float k = (float)(dt / (double)INPUT_CARET_GLIDE_TIME);
        if (k > 1.0f)
            k = 1.0f;
        float e = Anim_eval(ANIM_EASE_OUT, k);
        float next = (*inp).caretX + ((*inp).caretTargetX - (*inp).caretX) * e;
        if (next != (*inp).caretX) {
            (*inp).caretX = next;
            markDirty(inp);
        }
    }
}

void Input_free(Input *inp) {
    if (!inp)
        return;
    char *text = (*inp).text;
    if (text)
        Memory_free(text);
    char *holder = (*inp).placeholder;
    if (holder)
        Memory_free(holder);
    (*inp).text = nullptr;
    (*inp).placeholder = nullptr;
    (*inp).font = nullptr;
    (*inp).onChange = nullptr;
    (*inp).onSubmit = nullptr;
    (*inp).ctx = nullptr;
    (*inp).measurer = nullptr;
    (*inp).measureCtx = nullptr;
    Memory_free(inp);
}

// ============================================================================
// GETTERS
// ============================================================================

const char *Input_getText(const Input *inp) {
    return inp ? (*inp).text : nullptr;
}

size_t Input_getCap(const Input *inp) {
    return inp ? (*inp).cap : 0;
}

const char *Input_getPlaceholder(const Input *inp) {
    return inp ? (*inp).placeholder : nullptr;
}

bool Input_isPassword(const Input *inp) {
    return inp ? (*inp).password : false;
}

bool Input_isReadonly(const Input *inp) {
    return inp ? (*inp).readonly : false;
}

bool Input_isFocused(const Input *inp) {
    return inp ? (*inp).focused : false;
}

int32_t Input_getCursor(const Input *inp) {
    return inp ? (*inp).cursor : 0;
}

Font *Input_getFont(const Input *inp) {
    return inp ? (*inp).font : nullptr;
}

Input_ChangeFn Input_getOnChange(const Input *inp) {
    return inp ? (*inp).onChange : nullptr;
}

Input_SubmitFn Input_getOnSubmit(const Input *inp) {
    return inp ? (*inp).onSubmit : nullptr;
}

void *Input_getCtx(const Input *inp) {
    return inp ? (*inp).ctx : nullptr;
}

Input_MeasureFn Input_getMeasurer(const Input *inp) {
    return inp ? (*inp).measurer : nullptr;
}

void *Input_getMeasureContext(const Input *inp) {
    return inp ? (*inp).measureCtx : nullptr;
}

int Input_caret_getMode(const Input *inp) {
    return inp ? (*inp).caretMode : INPUT_CARET_BLINK;
}

uint32_t Input_caret_getColor(const Input *inp) {
    return inp ? (*inp).caretColor : 0xFFFFFFFFu;
}

float Input_caret_getBlinkPeriod(const Input *inp) {
    return inp ? (*inp).caretBlinkPeriod : INPUT_CARET_DEFAULT_PERIOD;
}

float Input_caret_getTarget(const Input *inp) {
    return inp ? (*inp).caretTargetX : 0.0f;
}

float Input_caret_getX(const Input *inp) {
    return inp ? (*inp).caretX : 0.0f;
}

bool Input_caret_isShown(const Input *inp) {
    return inp && (*inp).caretShown;
}

Panel *Input_caret_getView(const Input *inp) {
    return inp ? (*inp).caretView : nullptr;
}

float Input_caret_getOpacity(const Input *inp) {
    return inp ? (*inp).caretOpacity : 1.0f;
}

float Input_caret_getEffectiveOpacity(const Input *inp) {
    if (!inp || !(*inp).caretShown)
        return 0.0f;
    return (*inp).caretOpacity;
}
