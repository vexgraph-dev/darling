#ifndef DARLING_FIELD_INPUTOTP_H
#define DARLING_FIELD_INPUTOTP_H

#include <stdbool.h>
#include <stdint.h>

#include "c23/constructor.h"
#include "darling/panel/panel.h"

// Fixed-length one-time-code input: Panel layout plus an owned digit buffer.
typedef void (*InputOTP_CompleteFn)(void *ctx);

typedef struct InputOTP {
    Panel base;
    char *digits;
    int32_t length;
    float boxSize;
    InputOTP_CompleteFn onComplete;
    void *ctx;
} InputOTP;

InputOTP *InputOTP_1(int32_t length);
InputOTP *InputOTP_2(Panel *parent, int32_t length);
InputOTP *InputOTP_1_parent(Panel *parent);

#define InputOTP(...) CONSTRUCTOR_DISPATCH(InputOTP, __VA_ARGS__)

// Core entry (stub: append-and-fire lands with the caret walker).
void InputOTP_pushDigit(InputOTP *otp, char digit);

void InputOTP_free(InputOTP *otp);

void InputOTP_setDigits(InputOTP *otp, const char *digits);
void InputOTP_setBoxSize(InputOTP *otp, float size);
void InputOTP_setOnComplete(InputOTP *otp, InputOTP_CompleteFn fn);
void InputOTP_setCtx(InputOTP *otp, void *ctx);

const char *InputOTP_getDigits(const InputOTP *otp);
int32_t InputOTP_getLength(const InputOTP *otp);
float InputOTP_getBoxSize(const InputOTP *otp);
InputOTP_CompleteFn InputOTP_getOnComplete(const InputOTP *otp);
void *InputOTP_getCtx(const InputOTP *otp);

#endif
