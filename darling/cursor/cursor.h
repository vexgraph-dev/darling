#ifndef DARLING_CURSOR_H
#define DARLING_CURSOR_H

#include <stdbool.h>
#include <stdint.h>
#include "c23/constructor.h"

// Standard system cursor styles
typedef enum CursorType {
    CURSOR_DEFAULT       = 0,
    CURSOR_IBEAM         = 1,
    CURSOR_POINTING_HAND = 2,
    CURSOR_CROSSHAIR     = 3,
    CURSOR_RESIZE_EW     = 4,
    CURSOR_RESIZE_NS     = 5,
    CURSOR_NOT_ALLOWED   = 6,
    CURSOR_HIDDEN        = 7,
} CursorType;

typedef struct Cursor {
    int type;
    void *customData;
} Cursor;

Cursor *Cursor_0(void);
Cursor *Cursor_1(int type);

#define Cursor(...) CONSTRUCTOR_DISPATCH(Cursor, __VA_ARGS__)

Cursor *Cursor_getPredefined(int type);
void Cursor_free(Cursor *cursor);
void Cursor_apply(const Cursor *cursor, void *window);

// Symmetric Getters/Setters (Rule 24)
int Cursor_getType(const Cursor *cursor);
void Cursor_setType(Cursor *cursor, int type);
void *Cursor_getCustomData(const Cursor *cursor);
void Cursor_setCustomData(Cursor *cursor, void *customData);

#endif // DARLING_CURSOR_H
