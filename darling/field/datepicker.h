#ifndef DARLING_FIELD_DATEPICKER_H
#define DARLING_FIELD_DATEPICKER_H

#include <stdbool.h>
#include <stdint.h>

#include "c23/constructor.h"
#include "darling/panel/panel.h"

// Calendar date picker: Panel layout plus epoch-millis value and view month.
// Epoch millis keeps the node dependency-light (no datetime include).
typedef void (*DatePicker_PickFn)(void *ctx);

typedef struct DatePicker {
    Panel base;
    int64_t epochMillis;
    int32_t viewYear;
    int32_t viewMonth;
    DatePicker_PickFn onPick;
    void *ctx;
} DatePicker;

DatePicker *DatePicker_0(void);
DatePicker *DatePicker_1(Panel *parent);

#define DatePicker(...) CONSTRUCTOR_DISPATCH(DatePicker, __VA_ARGS__)

// Core clock read (stub: today computation lands with the clock walker).
void DatePicker_setToday(DatePicker *dp);

void DatePicker_setEpochMillis(DatePicker *dp, int64_t millis);
void DatePicker_setViewYear(DatePicker *dp, int32_t year);
void DatePicker_setViewMonth(DatePicker *dp, int32_t month);
void DatePicker_setView(DatePicker *dp, int32_t year, int32_t month);
void DatePicker_setOnPick(DatePicker *dp, DatePicker_PickFn fn);
void DatePicker_setCtx(DatePicker *dp, void *ctx);

int64_t DatePicker_getEpochMillis(const DatePicker *dp);
int32_t DatePicker_getViewYear(const DatePicker *dp);
int32_t DatePicker_getViewMonth(const DatePicker *dp);
DatePicker_PickFn DatePicker_getOnPick(const DatePicker *dp);
void *DatePicker_getCtx(const DatePicker *dp);

#endif
