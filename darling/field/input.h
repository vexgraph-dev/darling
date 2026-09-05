#ifndef DARLING_FIELD_INPUT_H
#define DARLING_FIELD_INPUT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "c23/constructor.h"
#include "darling/panel/panel.h"
#include "font/font.h"

// Single-line text input: Panel layout plus an owned bounded buffer.
typedef void (*Input_ChangeFn)(void *ctx);
typedef void (*Input_SubmitFn)(void *ctx);

typedef struct Input {
    Panel base;
    char *text;
    size_t cap;
    char *placeholder;
    bool password;
    bool readonly;
    int32_t cursor;
    Font *font;
    Input_ChangeFn onChange;
    Input_SubmitFn onSubmit;
    void *ctx;
} Input;

Input *Input_0(void);
Input *Input_2(Panel *parent, size_t cap);

#define Input(...) CONSTRUCTOR_DISPATCH(Input, __VA_ARGS__)

// Core editing (stubs: buffer surgery lands with the caret walker).
void Input_insertChar(Input *inp, char c);
void Input_eraseChar(Input *inp);

void Input_free(Input *inp);

void Input_setText(Input *inp, const char *text);
void Input_setCap(Input *inp, size_t cap);
void Input_setPlaceholder(Input *inp, const char *placeholder);
void Input_setPassword(Input *inp, bool password);
void Input_setReadonly(Input *inp, bool readonly);
void Input_setCursor(Input *inp, int32_t cursor);
void Input_setFont(Input *inp, Font *font);
void Input_setOnChange(Input *inp, Input_ChangeFn fn);
void Input_setOnSubmit(Input *inp, Input_SubmitFn fn);
void Input_setCtx(Input *inp, void *ctx);

const char *Input_getText(const Input *inp);
size_t Input_getCap(const Input *inp);
const char *Input_getPlaceholder(const Input *inp);
bool Input_isPassword(const Input *inp);
bool Input_isReadonly(const Input *inp);
int32_t Input_getCursor(const Input *inp);
Font *Input_getFont(const Input *inp);
Input_ChangeFn Input_getOnChange(const Input *inp);
Input_SubmitFn Input_getOnSubmit(const Input *inp);
void *Input_getCtx(const Input *inp);

#endif
