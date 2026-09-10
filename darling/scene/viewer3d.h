#ifndef DARLING_VIEWER3D_H
#define DARLING_VIEWER3D_H

#include <stdbool.h>
#include <stdint.h>

#include "c23/constructor.h"
#include "darling/panel/panel.h"
#include "annotation/intention.h"

;;INTENTION("Blender-style scene shell: Panel subclass with floor grid at origin, cube placeholder, VFS model path (.obj then .gltf), and scene-only camera. Never poses the object.")

#define VIEWER3D_PATH_MAX 512

// Forward declaration: the content slot borrows Object3D without pulling
// scene/object3d.h into every includer (no include cycle, Rule 18 intact).
typedef struct Object3D Object3D;

typedef struct Viewer3D {
    // --- Viewer core (viewport + scene/camera transform only) ---
    Panel base;
    bool showGrid;       // Floor grid at y = 0 (default true)
    float camYaw;        // Scene orbit yaw, radians
    float camPitch;      // Scene orbit pitch, radians
    float camDist;       // Dolly distance, > 0
    float panX;          // Scene pan X, world units
    float panY;          // Scene pan Y, world units
    // --- Content part (borrowed object slot; views only, never pierce) ---
    Object3D *object;    // BORROWED content (null = cube placeholder); detach-only
    // --- Source part (model path; VFS uri, empty = placeholder) ---
    char path[VIEWER3D_PATH_MAX]; // Inline model path, NUL-terminated
} Viewer3D;

Viewer3D *Viewer3D_0(void);
Viewer3D *Viewer3D_1(Panel *parent);

#define Viewer3D(...) CONSTRUCTOR_DISPATCH(Viewer3D, __VA_ARGS__)

void Viewer3D_setObject(Viewer3D *v, Object3D *object);
Object3D *Viewer3D_getObject(const Viewer3D *v);

void Viewer3D_setModelPath(Viewer3D *v, const char *path);
const char *Viewer3D_getModelPath(const Viewer3D *v);
void Viewer3D_clearModel(Viewer3D *v);

void Viewer3D_setShowGrid(Viewer3D *v, bool show);
bool Viewer3D_isShowingGrid(const Viewer3D *v);

void Viewer3D_setOrbit(Viewer3D *v, float yaw, float pitch);
void Viewer3D_getOrbit(const Viewer3D *v, float *outYaw, float *outPitch);

void Viewer3D_setDistance(Viewer3D *v, float dist);
float Viewer3D_getDistance(const Viewer3D *v);

void Viewer3D_setPan(Viewer3D *v, float x, float y);
void Viewer3D_getPan(const Viewer3D *v, float *outX, float *outY);

// Layout facade — inherit from Panel
static inline void Viewer3D_setLocation(Viewer3D *v, float x, float y)
    { if (v) Panel_setLocation(&(*v).base, x, y); }
static inline void Viewer3D_setSize(Viewer3D *v, float w, float h)
    { if (v) Panel_setSize(&(*v).base, w, h); }
static inline void Viewer3D_setAnchor(Viewer3D *v, int anchor)
    { if (v) Panel_setAnchor(&(*v).base, anchor); }
static inline void Viewer3D_setPivot(Viewer3D *v, int pivot)
    { if (v) Panel_setPivot(&(*v).base, pivot); }
static inline void Viewer3D_setBackgroundColor(Viewer3D *v, uint32_t color)
    { if (v) Panel_setBackgroundColor(&(*v).base, color); }

#endif
