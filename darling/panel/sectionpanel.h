#ifndef DARLING_SECTIONPANEL_H
#define DARLING_SECTIONPANEL_H

#include <stdbool.h>
#include <stdint.h>

#include "c23/constructor.h"
#include "darling/panel/panel.h"
#include "oop/type.h"

// darling/panel/sectionpanel.h — section-switching panel shell
// (a Panel whose children are sections; exactly one is current).

// Central registry owns these once landed; the guard keeps this shell
// compiling standalone until then.
#ifndef TYPE_SECTION_PANEL_SINGLETON
#define ID_SECTION_PANEL              0x0068u
#define TYPE_SECTION_PANEL_SINGLETON  (FORM_SINGLETON | ID_SECTION_PANEL)
#endif

typedef struct SectionPanel {
    Panel base;
    int32_t current;
    bool wrapAround;
    void (*onSectionChange)(void *ctx);
    void *ctx;
} SectionPanel;

// Constructors:
//   SectionPanel()         — detached shell, section 0
//   SectionPanel(parent)   — created and attached
SectionPanel *SectionPanel_0(void);
SectionPanel *SectionPanel_1(Panel *parent);

#define SectionPanel(...) CONSTRUCTOR_DISPATCH(SectionPanel, __VA_ARGS__)

// Selection advance (shell: clamps/wraps the index; full show/hide deferred).
void SectionPanel_next(SectionPanel *s);
void SectionPanel_prev(SectionPanel *s);

int32_t SectionPanel_getCurrent(const SectionPanel *s);
void SectionPanel_setCurrent(SectionPanel *s, int32_t index);
int32_t SectionPanel_getCount(const SectionPanel *s);
bool SectionPanel_getWrapAround(const SectionPanel *s);
void SectionPanel_setWrapAround(SectionPanel *s, bool wrap);

#endif
