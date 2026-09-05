#ifndef DARLING_EVENT_ACTION_H
#define DARLING_EVENT_ACTION_H

#include <stdbool.h>
#include <stdint.h>

#include "c23/constructor.h"
#include "darling/panel/panel.h"
#include "oop/type.h"

// event/action.h — transient semantic action message
// (button press, menu pick, dialog confirm).
//
// A plain struct, NOT a Panel: messages are never attached to the tree
// (no Darling_add arms; central wiring handles delivery). Every event
// carries nanos + consumed; consume() short-circuits the bubble walk.

// Central registry owns these once landed; the guard keeps this shell
// compiling standalone until then.
#ifndef TYPE_ACTION_EVENT_SINGLETON
#define ID_ACTION_EVENT 0x00A5u
#define TYPE_ACTION_EVENT_SINGLETON (FORM_SINGLETON | ID_ACTION_EVENT)
#endif

typedef struct ActionEvent {
    Panel *source;      // panel originating the action; nullptr = none yet
    int32_t actionId;   // numeric action id
    char *command;      // owned command string (e.g. "menu:file:open"); nullptr = none
    bool consumed;      // true = stop the bubble walk
    uint64_t nanos;     // plain settable timestamp (clock wiring is behavior phase)
} ActionEvent;

// Constructors:
//   ActionEvent()                  — empty shell, no command
//   ActionEvent(source, actionId)  — action from source with id
ActionEvent *ActionEvent_0(void);
ActionEvent *ActionEvent_2(Panel *source, int32_t actionId);

#define ActionEvent(...) CONSTRUCTOR_DISPATCH(ActionEvent, __VA_ARGS__)

// Core: Java-style consume, short-circuits the bubble walk later.
void ActionEvent_consume(ActionEvent *ev);

// Frees the owned command, then the event itself.
void ActionEvent_free(ActionEvent *ev);

Panel *ActionEvent_getSource(const ActionEvent *ev);
void ActionEvent_setSource(ActionEvent *ev, Panel *source);
int32_t ActionEvent_getActionId(const ActionEvent *ev);
void ActionEvent_setActionId(ActionEvent *ev, int32_t actionId);
const char *ActionEvent_getCommand(const ActionEvent *ev);
void ActionEvent_setCommand(ActionEvent *ev, const char *command);
uint64_t ActionEvent_getNanos(const ActionEvent *ev);
void ActionEvent_setNanos(ActionEvent *ev, uint64_t nanos);
bool ActionEvent_isConsumed(const ActionEvent *ev);

#endif
