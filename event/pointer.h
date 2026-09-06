#ifndef DARLING_EVENT_POINTER_H
#define DARLING_EVENT_POINTER_H

#include <stdbool.h>
#include <stdint.h>

#include "c23/constructor.h"
#include "darling/panel/panel.h"
#include "oop/type.h"

// event/pointer.h — transient pointer message (mouse/touch/stylus).
//
// A plain struct, NOT a Panel: messages are never attached to the tree
// (no Darling_add arms; central wiring handles delivery). Every event
// carries nanos + consumed; consume() short-circuits the bubble walk.

// Central registry owns these once landed; the guard keeps this shell
// compiling standalone until then.
#ifndef TYPE_POINTER_EVENT_SINGLETON
#define ID_POINTER_EVENT              0x00A2u
#define TYPE_POINTER_EVENT_SINGLETON  (FORM_SINGLETON | ID_POINTER_EVENT)
#endif

enum {
    PTR_DOWN = 0,
    PTR_MOVE,
    PTR_UP,
    PTR_DRAG,
    PTR_HOVER,
    PTR_ENTER,
    PTR_LEAVE,
    PTR_CANCEL
};

typedef struct PointerEvent {
    Panel *target;      // hit panel under the pointer; nullptr = none yet
    Panel *related;     // secondary panel (enter/leave pair); nullptr = none
    float x;            // pointer x in target-local points
    float y;            // pointer y in target-local points
    int32_t button;     // 0 none, 1 primary, 2 secondary, 3 middle
    float pressure;     // normalized 0.0 to 1.0; 0.0 = unknown
    int32_t kind;       // PTR_DOWN/MOVE/UP/DRAG/HOVER/ENTER/LEAVE/CANCEL
    uint64_t nanos;     // plain settable timestamp (clock wiring is behavior phase)
    int32_t phase;      // 0 capture, 1 target, 2 bubble
    bool consumed;      // true = stop the bubble walk
} PointerEvent;

// Constructors:
//   PointerEvent()                    — empty shell, kind PTR_DOWN, phase target
//   PointerEvent(kind, x, y, button)  — positioned press/move with button
PointerEvent *PointerEvent_0(void);
PointerEvent *PointerEvent_4(int32_t kind, float x, float y, int32_t button);

#define PointerEvent(...) CONSTRUCTOR_DISPATCH(PointerEvent, __VA_ARGS__)

// Core: Java-style consume, short-circuits the bubble walk later.
void PointerEvent_consume(PointerEvent *ev);

Panel *PointerEvent_getTarget(const PointerEvent *ev);
void PointerEvent_setTarget(PointerEvent *ev, Panel *target);
Panel *PointerEvent_getRelated(const PointerEvent *ev);
void PointerEvent_setRelated(PointerEvent *ev, Panel *related);
float PointerEvent_getX(const PointerEvent *ev);
void PointerEvent_setX(PointerEvent *ev, float x);
float PointerEvent_getY(const PointerEvent *ev);
void PointerEvent_setY(PointerEvent *ev, float y);
int32_t PointerEvent_getButton(const PointerEvent *ev);
void PointerEvent_setButton(PointerEvent *ev, int32_t button);
float PointerEvent_getPressure(const PointerEvent *ev);
void PointerEvent_setPressure(PointerEvent *ev, float pressure);
int32_t PointerEvent_getKind(const PointerEvent *ev);
void PointerEvent_setKind(PointerEvent *ev, int32_t kind);
uint64_t PointerEvent_getNanos(const PointerEvent *ev);
void PointerEvent_setNanos(PointerEvent *ev, uint64_t nanos);
int32_t PointerEvent_getPhase(const PointerEvent *ev);
void PointerEvent_setPhase(PointerEvent *ev, int32_t phase);
bool PointerEvent_isConsumed(const PointerEvent *ev);

#endif
