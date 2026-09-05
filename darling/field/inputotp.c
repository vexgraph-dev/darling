#include "darling/field/inputotp.h"

#include <string.h>

#include "annotation/incomplete.h"
#include "nio/mem.h"
#include "oop/type.h"
#include "annotation/overview.h"

;;OVERVIEW
/**
 * ============================================================================
 * CLASS: InputOTP (inherits Panel, LEVEL L2 Behavior)
 * ============================================================================
 * Fixed-length one-time-code shell: Panel layout plus an owned digit buffer
 * of length chars, per-box size, and a completion callback slot.
 *
 * STRUCT FIELDS (Mirroring darling/field/inputotp.h):
 * ----------------------------------------------------------------------------
 *   Panel base;                  // Inherited layout, bounds, hierarchy state
 *   char *digits;                // Owned digit buffer (length + 1, NUL ended)
 *   int32_t length;              // Fixed code length (buffer capacity)
 *   float boxSize;               // Per-digit box edge in points
 *   InputOTP_CompleteFn onComplete; // Full-code callback; nullptr = none
 *   void *ctx;                   // Callback context (borrowed)
 *
 * FUNCTION REGISTRY:
 * ----------------------------------------------------------------------------
 * Constructors:
 *   - InputOTP(length)          : InputOTP_1(length)
 *   - InputOTP(parent, length)  : InputOTP_2(parent, length)
 *   - InputOTP_1_parent(parent) : default length, attached (Label precedent)
 *
 * Core Functions:
 *   - InputOTP_pushDigit(otp, digit)
 *
 * Setters:
 *   - InputOTP_setDigits(otp, digits)
 *   - InputOTP_setBoxSize(otp, size)
 *   - InputOTP_setOnComplete(otp, fn)
 *   - InputOTP_setCtx(otp, ctx)
 *   - InputOTP_free(otp)
 *
 * Getters:
 *   - InputOTP_getDigits(otp)
 *   - InputOTP_getLength(otp)
 *   - InputOTP_getBoxSize(otp)
 *   - InputOTP_getOnComplete(otp)
 *   - InputOTP_getCtx(otp)
 * ============================================================================
 */

// ============================================================================
// CONSTRUCTORS
// ============================================================================

#define INPUTOTP_DEFAULT_LENGTH 6
#define INPUTOTP_DEFAULT_BOX 40.0f

static InputOTP *allocOtp(int32_t length) {
    if (length < 0)
        length = 0;
    InputOTP *otp = (InputOTP*) Memory_alloc(TYPE_INPUTOTP_SINGLETON, sizeof(InputOTP));
    if (!otp)
        return nullptr;
    Panel *bp = Panel_0();
    if (!bp) {
        Memory_free(otp);
        return nullptr;
    }
    (*otp).base = (*bp);
    Memory_free(bp);
    size_t cap = (size_t)length + 1;
    char *buf = (char*) Memory_alloc(TYPE_ARRAY, cap);
    if (!buf) {
        Memory_free(otp);
        return nullptr;
    }
    buf[0] = '\0';
    (*otp).digits = buf;
    (*otp).length = length;
    (*otp).boxSize = INPUTOTP_DEFAULT_BOX;
    (*otp).onComplete = nullptr;
    (*otp).ctx = nullptr;
    return otp;
}

InputOTP *InputOTP_1(int32_t length) {
    return allocOtp(length);
}

InputOTP *InputOTP_1_parent(Panel *parent) {
    InputOTP *otp = allocOtp(INPUTOTP_DEFAULT_LENGTH);
    if (otp && parent) {
        Panel *bp = &(*otp).base;
        Panel_addContainer(parent, bp);
    }
    return otp;
}

InputOTP *InputOTP_2(Panel *parent, int32_t length) {
    InputOTP *otp = allocOtp(length);
    if (otp && parent) {
        Panel *bp = &(*otp).base;
        Panel_addContainer(parent, bp);
    }
    return otp;
}

// ============================================================================
// CORE FUNCTIONS
// ============================================================================

void InputOTP_pushDigit(InputOTP *otp, char digit) {
    ;;INCOMPLETE // append-and-fire lands with the caret walker
    (void)otp;
    (void)digit;
}

// ============================================================================
// SETTERS
// ============================================================================

static void markDirty(InputOTP *otp) {
    if (!otp)
        return;
    Panel *bp = &(*otp).base;
    Container_markDirty(&(*bp).base);
}

void InputOTP_setDigits(InputOTP *otp, const char *digits) {
    if (!otp)
        return;
    char *buf = (*otp).digits;
    if (!buf)
        return;
    int32_t cap = (*otp).length;
    if (cap < 0)
        cap = 0;
    buf[0] = '\0';
    if (digits) {
        size_t len = strlen(digits);
        if (len > (size_t)cap)
            len = (size_t)cap;
        memcpy(buf, digits, len);
        buf[len] = '\0';
    }
    markDirty(otp);
}

void InputOTP_setBoxSize(InputOTP *otp, float size) {
    if (!otp)
        return;
    (*otp).boxSize = size;
    markDirty(otp);
}

void InputOTP_setOnComplete(InputOTP *otp, InputOTP_CompleteFn fn) {
    if (!otp)
        return;
    (*otp).onComplete = fn;
}

void InputOTP_setCtx(InputOTP *otp, void *ctx) {
    if (!otp)
        return;
    (*otp).ctx = ctx;
}

void InputOTP_free(InputOTP *otp) {
    if (!otp)
        return;
    char *digits = (*otp).digits;
    if (digits)
        Memory_free(digits);
    (*otp).digits = nullptr;
    (*otp).onComplete = nullptr;
    (*otp).ctx = nullptr;
    Memory_free(otp);
}

// ============================================================================
// GETTERS
// ============================================================================

const char *InputOTP_getDigits(const InputOTP *otp) {
    return otp ? (*otp).digits : nullptr;
}

int32_t InputOTP_getLength(const InputOTP *otp) {
    return otp ? (*otp).length : 0;
}

float InputOTP_getBoxSize(const InputOTP *otp) {
    return otp ? (*otp).boxSize : 0.0f;
}

InputOTP_CompleteFn InputOTP_getOnComplete(const InputOTP *otp) {
    return otp ? (*otp).onComplete : nullptr;
}

void *InputOTP_getCtx(const InputOTP *otp) {
    return otp ? (*otp).ctx : nullptr;
}
