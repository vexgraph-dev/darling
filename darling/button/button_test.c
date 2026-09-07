#include "annotation/overview.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#include "darling/button/button.h"
#include "event/pointer.h"

;;OVERVIEW
/**
 * ============================================================================
 * MODULE: ButtonTest (darling/button/button_test.c)
 * LEVEL: L3 — Module Code (headless verification harness)
 * ============================================================================
 * Headless suite for Button live click controls (Pkg 2): pointer hover,
 * down, up, leave, disabled state, inside/outside bounds, callback firing.
 *
 * STRUCT FIELDS: none — procedural test harness.
 *
 * FUNCTION REGISTRY:
 * ----------------------------------------------------------------------------
 * Core Functions:
 *   - main(void)
 * ============================================================================
 */

static void onButtonPress(void *ctx) {
    (*(bool*) ctx) = true;
}

int main(void) {
    printf("=== Running Button Test Suite ===\n");

    // §1 Null safety
    Button_handlePointer(nullptr, PTR_DOWN, 10.0f, 10.0f);
    Button_handlePointer(nullptr, PTR_UP, 10.0f, 10.0f);
    Button_press(nullptr);

    // §2 Instantiate widget + set callback with context pointer
    Button *b = Button_1("Click Me");
    assert(b != nullptr);

    bool fired = false;
    Button_setOnPress(b, onButtonPress, &fired);

    // §3 Call _handlePointer with PTR_DOWN then PTR_UP inside bounds
    // Default bounds are 80x30 when unset
    Button_handlePointer(b, PTR_DOWN, 20.0f, 15.0f);
    assert(Button_isPressed(b) == true);
    assert(fired == false);

    Button_handlePointer(b, PTR_UP, 20.0f, 15.0f);
    assert(Button_isPressed(b) == false);
    assert(fired == true);

    // §4 Call _handlePointer with PTR_UP outside bounds and assert callback did NOT fire
    fired = false;
    Button_handlePointer(b, PTR_DOWN, 20.0f, 15.0f);
    assert(Button_isPressed(b) == true);
    Button_handlePointer(b, PTR_UP, 200.0f, 200.0f);
    assert(Button_isPressed(b) == false);
    assert(fired == false);

    // Pure PTR_UP outside bounds with no prior down
    fired = false;
    Button_handlePointer(b, PTR_UP, 200.0f, 200.0f);
    assert(fired == false);

    // §5 Hover & Leave lifecycle
    assert(Button_isHovered(b) == false);
    Button_handlePointer(b, PTR_HOVER, 20.0f, 15.0f);
    assert(Button_isHovered(b) == true);
    Button_handlePointer(b, PTR_LEAVE, 0.0f, 0.0f);
    assert(Button_isHovered(b) == false);

    // §6 Disabled button suppresses down and press
    Button_setDisabled(b, true);
    fired = false;
    Button_handlePointer(b, PTR_DOWN, 20.0f, 15.0f);
    assert(Button_isPressed(b) == false);
    Button_handlePointer(b, PTR_UP, 20.0f, 15.0f);
    assert(fired == false);

    Button_free(b);
    printf("=== Button Test PASS ===\n");
    return 0;
}
