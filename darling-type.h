#ifndef DARLING_TYPE_H
#define DARLING_TYPE_H

#include "oop/type.h"

// darling-type.h — the darling project's type registry.
//
// OWNERSHIP: every darling class ID lives here, not in vexspoke's
// oop/type.h. vexspoke owns the layout (masks, forms, PROJ_*, ARCH_*,
// helpers); each project owns its class numbers. Included by
// darling/panel/panel.h, so the whole darling tree inherits it.
//
// CLASS-SPACE CONTRACT (Rule 17): darling classes live in 0x0065-0x00FF.
// All new darling IDs MUST land in that range — the central parent chain
// and arch switch in vexspoke oop/type.c dispatch on ranges and may not
// include this file (upstream never includes downstream), so out-of-range
// IDs silently misresolve. Vexspoke keeps 0x0001-0x0064 and 0x4000+.

// --- DARLING UI TREE (structural subclasses) ---
#define ID_CONTAINER              0x0079u
#define ID_PANEL                  0x0078u
#define ID_PICTURE                0x007Au
#define ID_LABEL                  0x007Bu
#define ID_SCENE                  0x007Cu
#define ID_SCENE2D                0x007Du
#define ID_SCENE3D                0x007Eu
#define ID_RICH_LABEL             0x007Fu
#define ID_CANVAS                 0x0065u

// Structural subclass singletons (darling tree)
#define TYPE_CONTAINER_SINGLETON  (PROJ_DARLING | FORM_SINGLETON | ID_CONTAINER)
#define TYPE_PANEL_SINGLETON      (PROJ_DARLING | FORM_SINGLETON | ID_PANEL)
#define TYPE_PICTURE_SINGLETON    (PROJ_DARLING | FORM_SINGLETON | ID_PICTURE)
#define TYPE_LABEL_SINGLETON      (PROJ_DARLING | FORM_SINGLETON | ID_LABEL)
#define TYPE_SCENE_SINGLETON      (PROJ_DARLING | FORM_SINGLETON | ID_SCENE)
#define TYPE_SCENE2D_SINGLETON    (PROJ_DARLING | FORM_SINGLETON | ID_SCENE2D)
#define TYPE_SCENE3D_SINGLETON    (PROJ_DARLING | FORM_SINGLETON | ID_SCENE3D)
#define TYPE_RICH_LABEL_SINGLETON (PROJ_DARLING | FORM_SINGLETON | ID_RICH_LABEL)
#define TYPE_CANVAS_SINGLETON     (PROJ_DARLING | FORM_SINGLETON | ID_CANVAS)

// --- DARLING COMPONENTS (panel/label/scene/picture families + inputs) ---
// One ID per component class; all descend from ID_PANEL (see
// Type_getParentClass). Shell-first: struct + accessors land before behavior.
#define ID_LAYERED_PANEL  0x0067u
#define ID_SECTION_PANEL  0x0068u
#define ID_BUTTON         0x0090u
#define ID_SWITCH         0x0091u
#define ID_CHECKBOX       0x0092u
#define ID_RADIOGROUP     0x0093u
#define ID_SLIDER         0x0094u
#define ID_KNOB           0x0095u
#define ID_INPUT          0x0096u
#define ID_TEXTAREA       0x0097u
#define ID_INPUTOTP       0x0098u
#define ID_SELECT         0x0099u
#define ID_DATEPICKER     0x009Au
#define ID_COLORPICKER    0x009Bu
#define ID_COLORSWATCH    0x009Cu
#define ID_FILEDIALOG     0x009Du
#define ID_COLORDIALOG    0x009Eu
#define ID_DIALOG         0x00A0u
#define ID_ALERTDIALOG    0x00A1u
#define ID_KBD            0x00B4u
#define ID_PLOT           0x00B9u
#define ID_TYPOGRAPHY     0x00C0u
#define ID_RICHTEXT       0x00C1u

// --- DARLING EVENTS (transient messages, not nodes: no Panel base,
// no attach arms; Type_arch still reports ARCH_DARLING) ---
#define ID_POINTER_EVENT  0x00A2u
#define ID_KEY_EVENT      0x00A3u
#define ID_FOCUS_EVENT    0x00A4u
#define ID_ACTION_EVENT   0x00A5u
#define ID_VALUE_EVENT    0x00A6u
#define ID_TREE_EVENT     0x00A7u
#define ID_GESTURE_EVENT  0x00A8u

#define TYPE_POINTER_EVENT_SINGLETON (PROJ_DARLING | FORM_SINGLETON | ID_POINTER_EVENT)
#define TYPE_KEY_EVENT_SINGLETON     (PROJ_DARLING | FORM_SINGLETON | ID_KEY_EVENT)
#define TYPE_FOCUS_EVENT_SINGLETON   (PROJ_DARLING | FORM_SINGLETON | ID_FOCUS_EVENT)
#define TYPE_ACTION_EVENT_SINGLETON  (PROJ_DARLING | FORM_SINGLETON | ID_ACTION_EVENT)
#define TYPE_VALUE_EVENT_SINGLETON   (PROJ_DARLING | FORM_SINGLETON | ID_VALUE_EVENT)
#define TYPE_TREE_EVENT_SINGLETON    (PROJ_DARLING | FORM_SINGLETON | ID_TREE_EVENT)
#define TYPE_GESTURE_EVENT_SINGLETON (PROJ_DARLING | FORM_SINGLETON | ID_GESTURE_EVENT)

#define TYPE_LAYERED_PANEL_SINGLETON (PROJ_DARLING | FORM_SINGLETON | ID_LAYERED_PANEL)
#define TYPE_SECTION_PANEL_SINGLETON (PROJ_DARLING | FORM_SINGLETON | ID_SECTION_PANEL)
#define TYPE_BUTTON_SINGLETON        (PROJ_DARLING | FORM_SINGLETON | ID_BUTTON)
#define TYPE_SWITCH_SINGLETON        (PROJ_DARLING | FORM_SINGLETON | ID_SWITCH)
#define TYPE_CHECKBOX_SINGLETON      (PROJ_DARLING | FORM_SINGLETON | ID_CHECKBOX)
#define TYPE_RADIOGROUP_SINGLETON    (PROJ_DARLING | FORM_SINGLETON | ID_RADIOGROUP)
#define TYPE_SLIDER_SINGLETON        (PROJ_DARLING | FORM_SINGLETON | ID_SLIDER)
#define TYPE_KNOB_SINGLETON          (PROJ_DARLING | FORM_SINGLETON | ID_KNOB)
#define TYPE_INPUT_SINGLETON         (PROJ_DARLING | FORM_SINGLETON | ID_INPUT)
#define TYPE_TEXTAREA_SINGLETON      (PROJ_DARLING | FORM_SINGLETON | ID_TEXTAREA)
#define TYPE_INPUTOTP_SINGLETON      (PROJ_DARLING | FORM_SINGLETON | ID_INPUTOTP)
#define TYPE_SELECT_SINGLETON        (PROJ_DARLING | FORM_SINGLETON | ID_SELECT)
#define TYPE_DATEPICKER_SINGLETON    (PROJ_DARLING | FORM_SINGLETON | ID_DATEPICKER)
#define TYPE_COLORPICKER_SINGLETON   (PROJ_DARLING | FORM_SINGLETON | ID_COLORPICKER)
#define TYPE_COLORSWATCH_SINGLETON   (PROJ_DARLING | FORM_SINGLETON | ID_COLORSWATCH)
#define TYPE_FILEDIALOG_SINGLETON    (PROJ_DARLING | FORM_SINGLETON | ID_FILEDIALOG)
#define TYPE_COLORDIALOG_SINGLETON   (PROJ_DARLING | FORM_SINGLETON | ID_COLORDIALOG)
#define TYPE_DIALOG_SINGLETON        (PROJ_DARLING | FORM_SINGLETON | ID_DIALOG)
#define TYPE_ALERTDIALOG_SINGLETON   (PROJ_DARLING | FORM_SINGLETON | ID_ALERTDIALOG)
#define TYPE_KBD_SINGLETON           (PROJ_DARLING | FORM_SINGLETON | ID_KBD)
#define TYPE_PLOT_SINGLETON          (PROJ_DARLING | FORM_SINGLETON | ID_PLOT)
#define TYPE_TYPOGRAPHY_SINGLETON    (PROJ_DARLING | FORM_SINGLETON | ID_TYPOGRAPHY)
#define TYPE_RICHTEXT_SINGLETON      (PROJ_DARLING | FORM_SINGLETON | ID_RICHTEXT)

#endif
