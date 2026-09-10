#ifndef DARLING_MATERIAL_PANEL_H
#define DARLING_MATERIAL_PANEL_H

#include <stdbool.h>
#include <stdint.h>

#include "c23/constructor.h"
#include "darling/panel/panel.h"
#include "annotation/intention.h"

;;INTENTION("Material lab node: Panel subclass binding a borrowed graphvex material to a borrowed preview object. Albedo/roughness/metallic part verbs; the panel owns no lighting state itself.")

// Forward declaration: preview target borrows Object3D without pulling
// scene/object3d.h into every includer (no include cycle, Rule 18 intact).
typedef struct Object3D Object3D;

typedef struct MaterialPanel {
    // --- Panel core (layout/tree/bg; the lab is a panel like any other) ---
    Panel base;
    // --- Material part (borrowed graphvex material; views only, never pierce) ---
    void *material;      // BORROWED material handle (null = default gray)
    // --- Preview part (borrowed target; views only, never pierce) ---
    Object3D *preview;   // BORROWED preview object (usually the viewer cube); detach-only
} MaterialPanel;

MaterialPanel *MaterialPanel_0(void);
MaterialPanel *MaterialPanel_1(Panel *parent);

#define MaterialPanel(...) CONSTRUCTOR_DISPATCH(MaterialPanel, __VA_ARGS__)

void MaterialPanel_setMaterial(MaterialPanel *m, void *material);
void *MaterialPanel_getMaterial(const MaterialPanel *m);

void MaterialPanel_setPreviewTarget(MaterialPanel *m, Object3D *preview);
Object3D *MaterialPanel_getPreviewTarget(const MaterialPanel *m);

// Part verbs over the bound material (live values, re-render on set):
void MaterialPanel_material_setAlbedo(MaterialPanel *m, uint32_t color);
uint32_t MaterialPanel_material_getAlbedo(const MaterialPanel *m);

void MaterialPanel_material_setRoughness(MaterialPanel *m, float v);
float MaterialPanel_material_getRoughness(const MaterialPanel *m);

void MaterialPanel_material_setMetallic(MaterialPanel *m, float v);
float MaterialPanel_material_getMetallic(const MaterialPanel *m);

// Layout facade — inherit from Panel
static inline void MaterialPanel_setLocation(MaterialPanel *m, float x, float y)
    { if (m) Panel_setLocation(&(*m).base, x, y); }
static inline void MaterialPanel_setSize(MaterialPanel *m, float w, float h)
    { if (m) Panel_setSize(&(*m).base, w, h); }
static inline void MaterialPanel_setAnchor(MaterialPanel *m, int anchor)
    { if (m) Panel_setAnchor(&(*m).base, anchor); }
static inline void MaterialPanel_setPivot(MaterialPanel *m, int pivot)
    { if (m) Panel_setPivot(&(*m).base, pivot); }
static inline void MaterialPanel_setBackgroundColor(MaterialPanel *m, uint32_t color)
    { if (m) Panel_setBackgroundColor(&(*m).base, color); }

#endif
