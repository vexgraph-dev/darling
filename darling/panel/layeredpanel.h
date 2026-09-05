#ifndef DARLING_LAYEREDPANEL_H
#define DARLING_LAYEREDPANEL_H

#include <stdbool.h>
#include <stdint.h>

#include "c23/constructor.h"
#include "darling/panel/panel.h"
#include "oop/type.h"

// darling/panel/layeredpanel.h — stacked-band panel shell
// (a Panel with a fixed band count plus a per-band visibility mask).

// Central registry owns these once landed; the guard keeps this shell
// compiling standalone until then.
#ifndef TYPE_LAYERED_PANEL_SINGLETON
#define ID_LAYERED_PANEL 0x0067u
#define TYPE_LAYERED_PANEL_SINGLETON (FORM_SINGLETON | ID_LAYERED_PANEL)
#endif

typedef struct LayeredPanel {
    Panel base;
    int32_t bandCount;
    uint32_t bandVisibleMask;
} LayeredPanel;

// Constructors:
//   LayeredPanel()         — detached shell, zero bands
//   LayeredPanel(parent)   — created and attached
LayeredPanel *LayeredPanel_0(void);
LayeredPanel *LayeredPanel_1(Panel *parent);

#define LayeredPanel(...) CONSTRUCTOR_DISPATCH(LayeredPanel, __VA_ARGS__)

int32_t LayeredPanel_getBandCount(const LayeredPanel *p);
void LayeredPanel_setBandCount(LayeredPanel *p, int32_t count);
void LayeredPanel_setBandVisible(LayeredPanel *p, int32_t band, bool visible);
bool LayeredPanel_isBandVisible(const LayeredPanel *p, int32_t band);

#endif
