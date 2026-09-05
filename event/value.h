#ifndef DARLING_EVENT_VALUE_H
#define DARLING_EVENT_VALUE_H

#include <stdbool.h>
#include <stdint.h>

#include "c23/constructor.h"
#include "darling/panel/panel.h"
#include "oop/type.h"

// event/value.h — transient value-change message
// (slider drag, checkbox toggle, text edit).
//
// A plain struct, NOT a Panel: messages are never attached to the tree
// (no Darling_add arms; central wiring handles delivery). Every event
// carries nanos + consumed; consume() short-circuits the bubble walk.

// Central registry owns these once landed; the guard keeps this shell
// compiling standalone until then.
#ifndef TYPE_VALUE_EVENT_SINGLETON
#define ID_VALUE_EVENT 0x00A6u
#define TYPE_VALUE_EVENT_SINGLETON (FORM_SINGLETON | ID_VALUE_EVENT)
#endif

typedef struct ValueEvent {
    Panel *source;      // panel whose value changed; nullptr = none yet
    int32_t tag;        // value slot tag
    int64_t oldV;       // previous integer value
    int64_t newV;       // current integer value
    double oldD;        // previous float value
    double newD;        // current float value
    bool consumed;      // true = stop the bubble walk
    uint64_t nanos;     // plain settable timestamp (clock wiring is behavior phase)
} ValueEvent;

// Constructors:
//   ValueEvent()                 — empty shell, zeroed old/new
//   ValueEvent(source, tag)      — value change from source for slot tag
ValueEvent *ValueEvent_0(void);
ValueEvent *ValueEvent_2(Panel *source, int32_t tag);

#define ValueEvent(...) CONSTRUCTOR_DISPATCH(ValueEvent, __VA_ARGS__)

// Core: Java-style consume, short-circuits the bubble walk later.
void ValueEvent_consume(ValueEvent *ev);

Panel *ValueEvent_getSource(const ValueEvent *ev);
void ValueEvent_setSource(ValueEvent *ev, Panel *source);
int32_t ValueEvent_getTag(const ValueEvent *ev);
void ValueEvent_setTag(ValueEvent *ev, int32_t tag);
int64_t ValueEvent_getOldV(const ValueEvent *ev);
void ValueEvent_setOldV(ValueEvent *ev, int64_t oldV);
int64_t ValueEvent_getNewV(const ValueEvent *ev);
void ValueEvent_setNewV(ValueEvent *ev, int64_t newV);
double ValueEvent_getOldD(const ValueEvent *ev);
void ValueEvent_setOldD(ValueEvent *ev, double oldD);
double ValueEvent_getNewD(const ValueEvent *ev);
void ValueEvent_setNewD(ValueEvent *ev, double newD);
uint64_t ValueEvent_getNanos(const ValueEvent *ev);
void ValueEvent_setNanos(ValueEvent *ev, uint64_t nanos);
bool ValueEvent_isConsumed(const ValueEvent *ev);

#endif
