#ifndef DARLING_OBJECT3D_H
#define DARLING_OBJECT3D_H

#include <stdbool.h>
#include <stdint.h>

#include "c23/constructor.h"
#include "darling/panel/panel.h"
#include "annotation/intention.h"

;;INTENTION("Turntable object node: Panel subclass holding a borrowed graphvex mesh with rotation-only transform and fit-never-stretch framing from panel bounds.")

typedef struct Object3D {
    // --- Object core (framing input + object rotation; no position, no scale) ---
    Panel base;
    void *mesh;          // BORROWED graphvex mesh handle (null = nothing to draw)
    float rotX;          // Object rotation about X, radians (the ONLY transform)
    float rotY;          // Object rotation about Y, radians
    bool autoFit;        // Refit framing on every resize (default true)
    float fitPadding;    // Margin inside panel, 0..1 (default ~0.1)
} Object3D;

Object3D *Object3D_0(void);
Object3D *Object3D_1(Panel *parent);

#define Object3D(...) CONSTRUCTOR_DISPATCH(Object3D, __VA_ARGS__)

void Object3D_setMesh(Object3D *o, void *mesh);
void *Object3D_getMesh(const Object3D *o);

void Object3D_setRotation(Object3D *o, float x, float y);
void Object3D_getRotation(const Object3D *o, float *outX, float *outY);

void Object3D_setAutoFit(Object3D *o, bool autoFit);
bool Object3D_isAutoFit(const Object3D *o);

void Object3D_setFitPadding(Object3D *o, float padding);
float Object3D_getFitPadding(const Object3D *o);

// Layout facade — inherit from Panel
static inline void Object3D_setLocation(Object3D *o, float x, float y)
    { if (o) Panel_setLocation(&(*o).base, x, y); }
static inline void Object3D_setSize(Object3D *o, float w, float h)
    { if (o) Panel_setSize(&(*o).base, w, h); }
static inline void Object3D_setParentAnchor(Object3D *o, int anchor)
    { if (o) Panel_setParentAnchor(&(*o).base, anchor); }
static inline void Object3D_setSelfAnchor(Object3D *o, int anchor)
    { if (o) Panel_setSelfAnchor(&(*o).base, anchor); }
static inline void Object3D_setBackgroundColor(Object3D *o, uint32_t color)
    { if (o) Panel_setBackgroundColor(&(*o).base, color); }

#endif
