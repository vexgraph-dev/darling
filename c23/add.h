#ifndef DARLING_ADD_H
#define DARLING_ADD_H

#include <stdint.h>

#include "c23/overload.h"
#include "darling/canvas.h"
#include "darling/container.h"
#include "darling/label.h"
#include "darling/panel.h"
#include "darling/picture.h"
#include "darling/rich_label.h"
#include "darling/scene.h"
#include "oop/type.h"

// darling/c23/add.h — unified add() for the darling tree (thin wrapper).
// Lives in darling's c23/ (this folder), included as "c23/add.h".
// Filename must NEVER collide with vexspoke's c23/*.h (constructor.h,
// overload.h, ...): both roots sit on the include path, so a duplicate
// name would shadow upstream by -I order. Same reason there is no
// darling oop/type.h — the registry stays single in vexspoke.
//
// Reuses vexspoke's c23/overload.h (ov_type_error for loud compile errors;
// math add/sub/mul/div untouched) and adds Panel-family dispatch on top:
// one name accepts Panel/Label/Picture/RichLabel/Scene/Scene2D/Scene3D as
// either parent or child. Container/Canvas hit ov_type_error at compile
// time: a Container is layout data (no Panel to attach), a Canvas is a
// projection (not a node).
//
// Two layers, matching the codebase split:
//   compile time (_Generic here): picks the caster by child pointer TYPE.
//   runtime (add.c Darling_addAny): validates by class ID with if-cases on
//     Type_arch + Type_isA, then routes to a single static addContainer().
// Base-pointer calls (Panel *child holding a Label) skip the generic and go
// straight to Darling_addAny with an explicit ID — same if-cases apply.
//
// Attach semantics (Java add()): reparents the live child via
// Panel_addContainer. Deep-copy stays Panel_add(parent, node) — unchanged.

// Runtime attach with explicit class id (null-safe no-op on mismatch).
void Darling_addAny(Panel *parent, void *child, uint32_t childClass);

// --- parent normalization (any Panel-derived pointer -> Panel *) ------------
static inline Panel *Darling_asPanel_Panel(Panel *p) { return p; }
static inline Panel *Darling_asPanel_Label(Label *p) { return (Panel*) (void*) p; }
static inline Panel *Darling_asPanel_Picture(Picture *p) { return (Panel*) (void*) p; }
static inline Panel *Darling_asPanel_RichLabel(RichLabel *p) { return (Panel*) (void*) p; }
static inline Panel *Darling_asPanel_Scene(Scene *s) { return &(*s).base; }

static inline Panel *Darling_asPanel_Scene2D(Scene2D *s) {
    Scene *m = &(*s).base;
    return &(*m).base;
}

static inline Panel *Darling_asPanel_Scene3D(Scene3D *s) {
    Scene *m = &(*s).base;
    return &(*m).base;
}

#define Darling_asPanel(p) _Generic((p), \
    Panel *: Darling_asPanel_Panel, \
    Label *: Darling_asPanel_Label, \
    Picture *: Darling_asPanel_Picture, \
    RichLabel *: Darling_asPanel_RichLabel, \
    Scene *: Darling_asPanel_Scene, \
    Scene2D *: Darling_asPanel_Scene2D, \
    Scene3D *: Darling_asPanel_Scene3D \
)(p)

// --- per-child attach shims (one per accepted child type) -------------------
static inline void Darling_addPanel(Panel *parent, Panel *child) {
    Darling_addAny(parent, (void*) child, ID_PANEL);
}

static inline void Darling_addLabel(Panel *parent, Label *child) {
    Darling_addAny(parent, (void*) child, ID_LABEL);
}

static inline void Darling_addPicture(Panel *parent, Picture *child) {
    Darling_addAny(parent, (void*) child, ID_PICTURE);
}

static inline void Darling_addRichLabel(Panel *parent, RichLabel *child) {
    Darling_addAny(parent, (void*) child, ID_RICH_LABEL);
}

static inline void Darling_addScene(Panel *parent, Scene *child) {
    Darling_addAny(parent, (void*) child, ID_SCENE);
}

static inline void Darling_addScene2D(Panel *parent, Scene2D *child) {
    Darling_addAny(parent, (void*) child, ID_SCENE2D);
}

static inline void Darling_addScene3D(Panel *parent, Scene3D *child) {
    Darling_addAny(parent, (void*) child, ID_SCENE3D);
}

// Darling-side type helpers (no registry fork: IDs + Type_* live in
// vexspoke's oop/type.h; these just map darling pointers to them).
// classOf: pointer TYPE -> class ID at compile time (for Darling_addAny).
// kindName: class ID -> short name for logging/debugging.
#define Darling_classOf(child) _Generic((child), \
    Panel *: ID_PANEL, \
    Label *: ID_LABEL, \
    Picture *: ID_PICTURE, \
    RichLabel *: ID_RICH_LABEL, \
    Scene *: ID_SCENE, \
    Scene2D *: ID_SCENE2D, \
    Scene3D *: ID_SCENE3D, \
    Container *: ID_CONTAINER, \
    Canvas *: ID_CANVAS \
)

const char *Darling_kindName(uint32_t classId);

// Unified attach: Darling_add(parent, child). Parent accepts the same 7
// types via Darling_asPanel; child picks the shim by pointer type.
// Container/Canvas children are compile errors (ov_type_error), never silent.
#define Darling_add(parent, child) _Generic((child), \
    Panel *: Darling_addPanel, \
    Label *: Darling_addLabel, \
    Picture *: Darling_addPicture, \
    RichLabel *: Darling_addRichLabel, \
    Scene *: Darling_addScene, \
    Scene2D *: Darling_addScene2D, \
    Scene3D *: Darling_addScene3D, \
    Container *: ov_type_error, \
    Canvas *: ov_type_error, \
    default: ov_type_error \
)(Darling_asPanel(parent), (child))

#endif
