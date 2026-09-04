#ifndef DARLING_COMPOSITOR_H
#define DARLING_COMPOSITOR_H

#include <stdbool.h>
#include <stdint.h>
#include "window/window.h"

// darling/compositor.h — Retained-mode UI compositor bridge to Vulkan swapchain.
//
// Bridges darling UI panels, IOSurface overlays, and 3D scenes onto the
// OS-stable window and Vulkan presentation pipeline.

// Initialize the darling compositor for the given window and register
// frame rendering callbacks on the Vulkan presentation engine.
void Darling_initCompositor(Window *window);

// Shutdown compositor modules and unregister frame callbacks.
void Darling_shutdownCompositor(void);

// Pre-frame callback invoked before swapchain acquisition (runs offscreen IOSurface passes).
void Darling_preFrame(Window *window, int drawW, int drawH, void *userdata);

// Frame rendering callback invoked by Vk_clearPresent inside active swapchain pass.
void Darling_renderFrame(void *cmdBuffer, int drawW, int drawH, void *userdata);

#endif
