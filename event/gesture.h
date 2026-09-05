#ifndef DARLING_EVENT_GESTURE_H
#define DARLING_EVENT_GESTURE_H

#include <stdbool.h>
#include <stdint.h>

#include "c23/constructor.h"
#include "darling/panel/panel.h"
#include "oop/type.h"

// event/gesture.h — transient gesture message
// (tap, double-tap, long-press, pinch, swipe).
//
// A plain struct, NOT a Panel: messages are never attached to the tree
// (no Darling_add arms; central wiring handles delivery). Every event
// carries nanos + consumed; consume() short-circuits the bubble walk.

// Central registry owns these once landed; the guard keeps this shell
// compiling standalone until then.
#ifndef TYPE_GESTURE_EVENT_SINGLETON
#define ID_GESTURE_EVENT 0x00A8u
#define TYPE_GESTURE_EVENT_SINGLETON (FORM_SINGLETON | ID_GESTURE_EVENT)
#endif

enum {
    GESTURE_TAP = 0,
    GESTURE_DOUBLE,
    GESTURE_LONGPRESS,
    GESTURE_PINCH,
    GESTURE_SWIPE
};

typedef struct GestureEvent {
    Panel *target;      // panel under the gesture; nullptr = none yet
    int32_t kind;       // GESTURE_TAP/DOUBLE/LONGPRESS/PINCH/SWIPE
    float x;            // gesture centroid x in target-local points
    float y;            // gesture centroid y in target-local points
    float scale;        // pinch scale factor; 1.0 = identity
    float rotation;     // rotation in radians; 0.0 = none
    int32_t touches;    // active touch count
    bool consumed;      // true = stop the bubble walk
    uint64_t nanos;     // plain settable timestamp (clock wiring is behavior phase)
} GestureEvent;

// Constructors:
//   GestureEvent()                — empty shell, tap with no touches
//   GestureEvent(kind, touches)   — gesture of kind with touch count
GestureEvent *GestureEvent_0(void);
GestureEvent *GestureEvent_2(int32_t kind, int32_t touches);

#define GestureEvent(...) CONSTRUCTOR_DISPATCH(GestureEvent, __VA_ARGS__)

// Core: Java-style consume, short-circuits the bubble walk later.
void GestureEvent_consume(GestureEvent *ev);

Panel *GestureEvent_getTarget(const GestureEvent *ev);
void GestureEvent_setTarget(GestureEvent *ev, Panel *target);
int32_t GestureEvent_getKind(const GestureEvent *ev);
void GestureEvent_setKind(GestureEvent *ev, int32_t kind);
float GestureEvent_getX(const GestureEvent *ev);
void GestureEvent_setX(GestureEvent *ev, float x);
float GestureEvent_getY(const GestureEvent *ev);
void GestureEvent_setY(GestureEvent *ev, float y);
float GestureEvent_getScale(const GestureEvent *ev);
void GestureEvent_setScale(GestureEvent *ev, float scale);
float GestureEvent_getRotation(const GestureEvent *ev);
void GestureEvent_setRotation(GestureEvent *ev, float rotation);
int32_t GestureEvent_getTouches(const GestureEvent *ev);
void GestureEvent_setTouches(GestureEvent *ev, int32_t touches);
uint64_t GestureEvent_getNanos(const GestureEvent *ev);
void GestureEvent_setNanos(GestureEvent *ev, uint64_t nanos);
bool GestureEvent_isConsumed(const GestureEvent *ev);

#endif
