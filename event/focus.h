#ifndef DARLING_EVENT_FOCUS_H
#define DARLING_EVENT_FOCUS_H

#include <stdbool.h>
#include <stdint.h>

#include "c23/constructor.h"
#include "darling/panel/panel.h"
#include "oop/type.h"

// event/focus.h — transient focus message (gain/loss of keyboard focus).
//
// A plain struct, NOT a Panel: messages are never attached to the tree
// (no Darling_add arms; central wiring handles delivery). Every event
// carries nanos + consumed; consume() short-circuits the bubble walk.

// Central registry owns these once landed; the guard keeps this shell
// compiling standalone until then.
#ifndef TYPE_FOCUS_EVENT_SINGLETON
#define ID_FOCUS_EVENT 0x00A4u
#define TYPE_FOCUS_EVENT_SINGLETON (FORM_SINGLETON | ID_FOCUS_EVENT)
#endif

typedef struct FocusEvent {
    Panel *target;      // panel gaining or losing focus; nullptr = none yet
    Panel *opposite;    // panel on the other side of the move; nullptr = none
    bool gained;        // true = gained focus, false = lost focus
    bool consumed;      // true = stop the bubble walk
    uint64_t nanos;     // plain settable timestamp (clock wiring is behavior phase)
} FocusEvent;

// Constructors:
//   FocusEvent()                 — empty shell, lost-focus shape
//   FocusEvent(target, gained)   — focus gain/loss for target
FocusEvent *FocusEvent_0(void);
FocusEvent *FocusEvent_2(Panel *target, bool gained);

#define FocusEvent(...) CONSTRUCTOR_DISPATCH(FocusEvent, __VA_ARGS__)

// Core: Java-style consume, short-circuits the bubble walk later.
void FocusEvent_consume(FocusEvent *ev);

Panel *FocusEvent_getTarget(const FocusEvent *ev);
void FocusEvent_setTarget(FocusEvent *ev, Panel *target);
Panel *FocusEvent_getOpposite(const FocusEvent *ev);
void FocusEvent_setOpposite(FocusEvent *ev, Panel *opposite);
bool FocusEvent_isGained(const FocusEvent *ev);
void FocusEvent_setGained(FocusEvent *ev, bool gained);
uint64_t FocusEvent_getNanos(const FocusEvent *ev);
void FocusEvent_setNanos(FocusEvent *ev, uint64_t nanos);
bool FocusEvent_isConsumed(const FocusEvent *ev);

#endif
