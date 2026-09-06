#ifndef DARLING_EVENT_TREE_H
#define DARLING_EVENT_TREE_H

#include <stdbool.h>
#include <stdint.h>

#include "c23/constructor.h"
#include "darling/panel/panel.h"
#include "oop/type.h"

// event/tree.h — transient tree-structure message (child added/removed).
//
// A plain struct, NOT a Panel: messages are never attached to the tree
// (no Darling_add arms; central wiring handles delivery). Every event
// carries nanos + consumed; consume() short-circuits the bubble walk.

// Central registry owns these once landed; the guard keeps this shell
// compiling standalone until then.
#ifndef TYPE_TREE_EVENT_SINGLETON
#define ID_TREE_EVENT              0x00A7u
#define TYPE_TREE_EVENT_SINGLETON  (FORM_SINGLETON | ID_TREE_EVENT)
#endif

typedef struct TreeEvent {
    Panel *parent;      // parent gaining or losing the child; nullptr = none yet
    Panel *child;       // child added or removed; nullptr = none yet
    bool added;         // true = added, false = removed
    bool consumed;      // true = stop the bubble walk
    uint64_t nanos;     // plain settable timestamp (clock wiring is behavior phase)
} TreeEvent;

// Constructors:
//   TreeEvent()                        — empty shell, removed shape
//   TreeEvent(parent, child, added)    — child added to / removed from parent
TreeEvent *TreeEvent_0(void);
TreeEvent *TreeEvent_3(Panel *parent, Panel *child, bool added);

#define TreeEvent(...) CONSTRUCTOR_DISPATCH(TreeEvent, __VA_ARGS__)

// Core: Java-style consume, short-circuits the bubble walk later.
void TreeEvent_consume(TreeEvent *ev);

Panel *TreeEvent_getParent(const TreeEvent *ev);
void TreeEvent_setParent(TreeEvent *ev, Panel *parent);
Panel *TreeEvent_getChild(const TreeEvent *ev);
void TreeEvent_setChild(TreeEvent *ev, Panel *child);
bool TreeEvent_isAdded(const TreeEvent *ev);
void TreeEvent_setAdded(TreeEvent *ev, bool added);
uint64_t TreeEvent_getNanos(const TreeEvent *ev);
void TreeEvent_setNanos(TreeEvent *ev, uint64_t nanos);
bool TreeEvent_isConsumed(const TreeEvent *ev);

#endif
