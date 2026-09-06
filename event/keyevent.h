#ifndef DARLING_EVENT_KEY_H
#define DARLING_EVENT_KEY_H

#include <stdbool.h>
#include <stdint.h>

#include "c23/constructor.h"
#include "darling/panel/panel.h"
#include "oop/type.h"

// event/keyevent.h — transient key message in the darling UI domain.
//
// NOTE: vexspoke's event/keyhandler.h defines a raw-input vtable named
// KeyHandler. Mechanism lives there (polling, slots, dispatch); the
// UI-domain event data lives here as UIKeyEvent. The split is
// deliberate: hardware in vexspoke, GUI in darling.
//
// A plain struct, NOT a Panel: messages are never attached to the tree
// (no Darling_add arms; central wiring handles delivery). Every event
// carries nanos + consumed; consume() short-circuits the bubble walk.

// Central registry owns these once landed; the guard keeps this shell
// compiling standalone until then.
#ifndef TYPE_KEY_EVENT_SINGLETON
#define ID_KEY_EVENT              0x00A3u
#define TYPE_KEY_EVENT_SINGLETON  (FORM_SINGLETON | ID_KEY_EVENT)
#endif

typedef struct UIKeyEvent {
    Panel *target;      // focused panel receiving the key; nullptr = none yet
    int32_t keyCode;    // platform key code
    int32_t ch;         // decoded codepoint; -1 = none
    uint32_t mods;      // modifier bitmask
    bool pressed;       // true = press, false = release
    bool repeat;        // true = auto-repeat
    bool consumed;      // true = stop the bubble walk
    uint64_t nanos;     // plain settable timestamp (clock wiring is behavior phase)
} UIKeyEvent;

// Constructors:
//   UIKeyEvent()                          — empty shell, ch -1, released
//   UIKeyEvent(keyCode, pressed, repeat)  — key press/release with repeat flag
UIKeyEvent *UIKeyEvent_0(void);
UIKeyEvent *UIKeyEvent_3(int32_t keyCode, bool pressed, bool repeat);

#define UIKeyEvent(...) CONSTRUCTOR_DISPATCH(UIKeyEvent, __VA_ARGS__)

// Core: Java-style consume, short-circuits the bubble walk later.
void UIKeyEvent_consume(UIKeyEvent *ev);

Panel *UIKeyEvent_getTarget(const UIKeyEvent *ev);
void UIKeyEvent_setTarget(UIKeyEvent *ev, Panel *target);
int32_t UIKeyEvent_getKeyCode(const UIKeyEvent *ev);
void UIKeyEvent_setKeyCode(UIKeyEvent *ev, int32_t keyCode);
int32_t UIKeyEvent_getCh(const UIKeyEvent *ev);
void UIKeyEvent_setCh(UIKeyEvent *ev, int32_t ch);
uint32_t UIKeyEvent_getMods(const UIKeyEvent *ev);
void UIKeyEvent_setMods(UIKeyEvent *ev, uint32_t mods);
bool UIKeyEvent_isPressed(const UIKeyEvent *ev);
void UIKeyEvent_setPressed(UIKeyEvent *ev, bool pressed);
bool UIKeyEvent_isRepeat(const UIKeyEvent *ev);
void UIKeyEvent_setRepeat(UIKeyEvent *ev, bool repeat);
uint64_t UIKeyEvent_getNanos(const UIKeyEvent *ev);
void UIKeyEvent_setNanos(UIKeyEvent *ev, uint64_t nanos);
bool UIKeyEvent_isConsumed(const UIKeyEvent *ev);

#endif
