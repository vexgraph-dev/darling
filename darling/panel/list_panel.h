#ifndef DARLING_LIST_PANEL_H
#define DARLING_LIST_PANEL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "c23/constructor.h"
#include "darling/panel/panel.h"
#include "oop/type.h"

// darling/panel/list_panel.h — indexed panel stack (a Panel whose children
// are ordered rows; index IS the API: append/insert/get/remove by index).
// Children stay ordinary Panels stored in the embedded base's child list
// (Panel_addContainer / Panel_getChild / Panel_removeChild) — no second
// child list. Bubbles are just child panels with radius + margin via the
// Phase-1 substrate (Panel_setMargin, Panel_setRadius). Detach-only: the
// list never frees children.

// Central registry owns these once landed; the guard keeps this shell
// compiling standalone until then.
#ifndef ID_LIST_PANEL
#define ID_LIST_PANEL 0x006Bu
#endif
#ifndef TYPE_LIST_PANEL_SINGLETON
#define TYPE_LIST_PANEL_SINGLETON (FORM_SINGLETON | ID_LIST_PANEL)
#endif

#define LIST_PANEL_VERTICAL    0
#define LIST_PANEL_HORIZONTAL  1

typedef struct ListPanel {
    Panel base;
    int32_t direction;
    float spacing;
    bool fillCross;
} ListPanel;

// Constructors:
//   ListPanel()            — vertical stack, zero spacing, no cross fill
//   ListPanel(direction)   — LIST_PANEL_VERTICAL / LIST_PANEL_HORIZONTAL
ListPanel *ListPanel_0(void);
ListPanel *ListPanel_1(int32_t direction);

#define ListPanel(...) CONSTRUCTOR_DISPATCH(ListPanel, __VA_ARGS__)

// Layout facade (same pattern as Panel_* shims: forward over the prefix).
static inline void ListPanel_setLocation(ListPanel *lp, float x, float y)
    { if (lp) Panel_setLocation(&(*lp).base, x, y); }
static inline void ListPanel_setSize(ListPanel *lp, float w, float h)
    { if (lp) Panel_setSize(&(*lp).base, w, h); }

// Core (detach-only: add/insert attach, remove detaches, never frees;
// every mutation re-runs the layout pass).
void ListPanel_add(ListPanel *lp, Panel *child);
void ListPanel_insert(ListPanel *lp, int32_t index, Panel *child);
Panel *ListPanel_get(const ListPanel *lp, int32_t index);
bool ListPanel_remove(ListPanel *lp, int32_t index);
size_t ListPanel_count(const ListPanel *lp);
void ListPanel_layout(ListPanel *lp);

// Setters.
void ListPanel_setSpacing(ListPanel *lp, float spacing);
void ListPanel_setDirection(ListPanel *lp, int32_t direction);
void ListPanel_setFillCross(ListPanel *lp, bool fill);

// Getters.
float ListPanel_getSpacing(const ListPanel *lp);
int32_t ListPanel_getDirection(const ListPanel *lp);
bool ListPanel_isFillCross(const ListPanel *lp);

#endif
