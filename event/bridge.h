#ifndef DARLING_EVENT_BRIDGE_H
#define DARLING_EVENT_BRIDGE_H

#include "darling/panel/panel.h"

// event/bridge.h — raw pipeline to dispatch handoff (struct-less MODULE).
//
// Attaches vexspoke Key/Mouse global listeners once and translates each
// raw callback into a darling UI event that is fired immediately:
//   key down/up/repeat -> UIKeyEvent (nanos = exactNanos from the ring)
//   mouse down/up      -> PointerEvent DOWN/UP (nanos = exactNanos)
//   mouse move/drag    -> PointerEvent MOVE/DRAG (stamped at delivery;
//                          motion is never judgment-critical)
// Char, scroll, zoom, delta, and touch arrive in a later turn.
// Focus is explicit (no auto-focus yet): the app sets the key target.

// Bind the tree root for pointer delivery + install listeners (idempotent).
void Darling_bridgeAttach(Panel *root);

// Remove listeners, clear root and focus.
void Darling_bridgeDetach(void);

// Explicit key target for Darling_fireKey (nullptr = keys go nowhere).
void Darling_bridgeSetFocused(Panel *p);
Panel *Darling_bridgeGetFocused(void);

// Tree root used for pointer delivery (nullptr when detached).
Panel *Darling_bridgeGetRoot(void);

#endif
